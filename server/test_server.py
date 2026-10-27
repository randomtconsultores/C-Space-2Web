from pathlib import Path
import sqlite3
import tempfile
import unittest
from .app import create_app
from .db import init_database


class ServerTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        root = Path(self.tmp.name)
        self.public = root / "public"
        self.public.mkdir()
        (self.public / "index.html").write_text("game page")
        self.database = root / "private" / "game.sqlite3"
        self.app = create_app({"TESTING": True, "DATABASE": self.database, "WEB_ROOT": self.public})
        self.client = self.app.test_client()

    def test_health_does_not_initialize(self):
        response = self.client.get("/api/v1/health")
        self.assertEqual(response.status_code, 503)
        self.assertEqual(response.json, {"error": {"code": "storage_unavailable"}})
        self.assertEqual(response.headers["Cache-Control"], "no-store")
        self.assertFalse(self.database.exists())
        self.assertNotIn(str(self.database), response.get_data(as_text=True))

    def test_explicit_cli_and_health(self):
        runner = self.app.test_cli_runner()
        self.assertEqual(runner.invoke(args=["init-db"]).exit_code, 0)
        self.assertEqual(runner.invoke(args=["init-db"]).exit_code, 0)
        response = self.client.get("/api/v1/health")
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.json, {"status": "ok", "schema_version": 1})
        self.assertEqual(response.headers["Cache-Control"], "no-store")
        self.assertEqual(self.client.post("/api/v1/health").status_code, 405)

    def test_storage_failure_is_generic(self):
        self.database.parent.mkdir()
        self.database.write_bytes(b"not sqlite")
        self.assertEqual(self.client.get("/api/v1/health").status_code, 503)
        self.database.unlink()
        init_database(self.database)
        with sqlite3.connect(self.database) as con:
            con.execute("DROP TABLE checkpoints")
        self.assertEqual(self.client.get("/api/v1/health").status_code, 503)

    def test_static_boundary_and_missing_routes(self):
        init_database(self.database)
        self.assertEqual(self.client.get("/").get_data(as_text=True), "game page")
        self.assertEqual(self.client.get("/../private/game.sqlite3").status_code, 404)
        self.assertEqual(self.client.get("/api/v1/checkpoint").status_code, 404)
        for method in ("POST", "PUT", "DELETE"):
            self.assertIn(self.client.open("/api/v1/checkpoint", method=method).status_code, (404, 405))
        try:
            (self.public / "leak.sqlite3").symlink_to(self.database)
        except (OSError, NotImplementedError):
            pass
        else:
            self.assertEqual(self.client.get("/leak.sqlite3").status_code, 404)

    def test_refuse_public_database_configuration(self):
        with self.assertRaises(ValueError):
            create_app({"DATABASE": self.public / "bad.sqlite3", "WEB_ROOT": self.public})
        with self.assertRaises(ValueError):
            create_app({"DATABASE": "relative.sqlite3", "WEB_ROOT": self.public})


if __name__ == "__main__":
    unittest.main()
