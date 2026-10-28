from contextlib import closing
import concurrent.futures
import os
from pathlib import Path
import sqlite3
import stat
import tempfile
import unittest
from unittest.mock import patch
from . import db


class DatabaseTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name) / "private" / "game.sqlite3"

    def test_init_is_idempotent(self):
        db.init_database(self.path)
        db.init_database(self.path)
        db.check_database(self.path)
        with db.connection(self.path) as con:
            self.assertEqual(con.execute("PRAGMA application_id").fetchone()[0], db.APP_ID)
            self.assertEqual(con.execute("PRAGMA user_version").fetchone()[0], 1)
            self.assertEqual(con.execute("PRAGMA foreign_keys").fetchone()[0], 1)

    def test_read_does_not_create(self):
        with self.assertRaises(sqlite3.Error):
            db.check_database(self.path)
        self.assertFalse(self.path.exists())
        self.assertFalse(self.path.parent.exists())

    def test_constraints_and_ownership(self):
        db.init_database(self.path)
        with db.connection(self.path) as con:
            con.execute("INSERT INTO sessions VALUES (?, ?, ?)", ("a" * 64, 1, 10))
            con.execute("INSERT INTO checkpoints VALUES (?, 1, 3, 2, 100, 1, 1)", ("a" * 64,))
            for column, value in [("lives", 0), ("lives", 10), ("score", -1),
                                  ("score", 2147473648), ("level_index", 4),
                                  ("format_version", 2), ("revision", 0)]:
                with self.assertRaises(sqlite3.IntegrityError):
                    con.execute("UPDATE checkpoints SET " + column + "=?", (value,))
            with self.assertRaises(sqlite3.IntegrityError):
                con.execute("INSERT INTO checkpoints VALUES (?, 1, 0, 3, 0, 1, 1)", ("b" * 64,))
            con.execute("DELETE FROM sessions")
            self.assertEqual(con.execute("SELECT count(*) FROM checkpoints").fetchone()[0], 0)

    def test_future_schema_is_preserved(self):
        db.init_database(self.path)
        with db.connection(self.path) as con:
            con.execute("PRAGMA user_version=99")
        before = self.path.read_bytes()
        with self.assertRaises(db.StoreError):
            db.init_database(self.path)
        self.assertEqual(self.path.read_bytes(), before)

    def test_foreign_database_is_preserved(self):
        self.path.parent.mkdir()
        with closing(sqlite3.connect(self.path)) as con:
            con.execute("CREATE TABLE precious (value TEXT)")
            con.execute("INSERT INTO precious VALUES ('keep')")
            con.commit()
        before = self.path.read_bytes()
        with self.assertRaises(db.StoreError):
            db.init_database(self.path)
        self.assertEqual(self.path.read_bytes(), before)

    def test_failed_init_rolls_back(self):
        with patch.object(db, "DDL", db.DDL + ("INVALID SQL",)):
            with self.assertRaises(sqlite3.Error):
                db.init_database(self.path)
        with db.connection(self.path) as con:
            self.assertEqual(con.execute("SELECT count(*) FROM sqlite_master").fetchone()[0], 0)
            self.assertEqual(con.execute("PRAGMA application_id").fetchone()[0], 0)
        db.init_database(self.path)
        db.check_database(self.path)

    def test_concurrent_init(self):
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
            list(pool.map(db.init_database, [self.path, self.path]))
        db.check_database(self.path)

    def test_uri_characters_permissions_and_absolute_path(self):
        special = Path(self.tmp.name) / "private ?#%.sqlite3"
        db.init_database(special)
        db.check_database(special)
        if os.name == "posix":
            self.assertEqual(stat.S_IMODE(special.stat().st_mode), 0o600)
        with self.assertRaises(ValueError):
            db.init_database("relative.sqlite3")


if __name__ == "__main__":
    unittest.main()
