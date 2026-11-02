#ifndef WEB_PERSIST_BRIDGE_H
#define WEB_PERSIST_BRIDGE_H

#include "space.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Inicializa o módulo da ponte. Deve ser chamado uma vez no arranque. */
void web_persist_bridge_init(void);

/* Limpa estado interno (exceto contador de tickets e disponibilidade). Chamado no encerramento. */
void web_persist_bridge_cleanup(void);

/* Notifica a disponibilidade da ponte JS→C. 1 = disponível, 0 = indisponível.
   Sobrevive a init()/cleanup(). Chamado por JS após init(Module). */
void web_persist_bridge_set_available(int available);

/* Devolve 1 se a ponte JS está disponível, 0 caso contrário. */
int web_persist_bridge_available(void);

/* ===== 5 wrappers escalares (fronteira JS/Wasm) ===== */

/* Pedido pendente: 0 = nenhum, 1 = LOAD, 2 = SAVE.
   Devolve 0 em PB_IDLE ou PB_RESULT_READY. */
int32_t web_persist_kind(void);

/* Ticket do pedido pendente (0 = nenhum).
   Devolve 0 em PB_IDLE ou PB_RESULT_READY. */
int32_t web_persist_ticket(void);

/* Snapshot privado capturado pelo C.
   field: 0=versão, 1=nível, 2=vidas, 3=pontuação.
   Retorna -1 se ticket/campo inválido ou se não houver SAVE pendente. */
int32_t web_persist_field(int32_t ticket, int32_t field);

/* Resultado assíncrono devolvido pelo JS.
   Os campos do checkpoint são usados no resultado de LOAD.
   Retorna 1 se consumido com sucesso, 0 se ticket inválido/contexto errado.
   Outcomes válidos:
     1  = sucesso
     0  = não encontrado
    -1  = erro
    -2  = snapshot inválido
    -3  = cancelado
    -4  = nível indisponível (level 3 com comprimento_mapa=0) */
int32_t web_persist_complete(int32_t ticket, int32_t outcome,
                             int32_t format_version, int32_t level_index,
                             int32_t lives, int32_t score);

/* Cancelamento explícito do pedido em curso */
void web_persist_cancel(void);

/* ===== Funções auxiliares para o menu/transição (chamadas pelo C) ===== */

/* Inicia pedido de LOAD. Retorna ticket (>0) ou 0 se falha/já ocupado. */
int32_t web_persist_begin_load(void);

/* Inicia pedido de SAVE. Captura snapshot do player atual via web_checkpoint_capture.
   Retorna ticket (>0) ou 0 se falha/já ocupado/snapshot inválido. */
int32_t web_persist_begin_save(const Player *player, int next_level);

/* ===== API para main.c obter resultado uma única vez ===== */
/* kind: 1=LOAD, 2=SAVE. Preenche *out_player e *out_level apenas se outcome==1.
   Retorna 1 se havia resultado pronto e foi consumido, 0 caso contrário. */
int web_persist_bridge_take_result(int *out_kind, int *out_outcome,
                                   Player *out_player, int *out_level);

#ifdef WEB_PERSIST_TEST
/* Apenas para testes: define o contador de tickets (não exportado no Wasm). */
void web_persist_bridge_test_set_counter(int32_t value);
#endif

#ifdef __cplusplus
}
#endif

#endif /* WEB_PERSIST_BRIDGE_H */
