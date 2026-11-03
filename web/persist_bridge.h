#ifndef WEB_PERSIST_BRIDGE_H
#define WEB_PERSIST_BRIDGE_H

#include "space.h"
#include <stdint.h>

#define WEB_SCORE_MAX 9
#define WEB_SCORE_NAME 8

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

/* ===== Leitura do ranking (kind 4) ===== */

/* Inicia pedido de leitura do ranking. Retorna ticket (>0) ou 0 se falha/já ocupado. */
int32_t web_persist_begin_score_list(void);

/* Devolve 1 se há resultado pronto a ser consumido (estado PB_RESULT_READY). */
int web_persist_result_ready(void);

/* Acessores ao ranking recebido (kind 4). Validos enquanto o pedido esta
   pendente ou o resultado pronto. Retornam -1 se ticket/index invalido. */
int32_t web_persist_score_count(int32_t ticket);
int32_t web_persist_score_position(int32_t ticket, int32_t index);
int32_t web_persist_score_score(int32_t ticket, int32_t index);
int32_t web_persist_score_name_char(int32_t ticket, int32_t index,
                                    int32_t char_index);

/* Alimentacao do ranking pelo JS (kind 4). Retorna 1 se aceito, 0 se invalido. */
int32_t web_persist_score_feed_entry(int32_t ticket, int32_t index,
                                     int32_t position, int32_t score);
int32_t web_persist_score_feed_name_char(int32_t ticket, int32_t index,
                                         int32_t char_index, int32_t char_code);

/* ===== Submissao de pontuacao (kind 3) ===== */

/* Inicia submissao de pontuacao com o nome no buffer de entrada (NAME_ENTRY).
   Retorna ticket (>0) ou 0 se falha/já ocupado/nome vazio. */
int32_t web_persist_begin_score(long score);

/* Acessores da submissao (kind 3). Retornam -1 se invalido. */
int32_t web_persist_submit_name_char(int32_t ticket, int32_t char_index);
int32_t web_persist_submit_score(int32_t ticket);
int32_t web_persist_submit_position(int32_t ticket);

/* ===== Buffer de introducao do nome (sincrono, fase NAME_ENTRY) ===== */

/* Alimenta um caractere: 1=aceito (acrescido/retrocedido),
   2=Enter confirmado (nome nao vazio), -1=ESC cancelado, 0=ignorado. */
int web_persist_name_feed(int ch);

/* Devolve o comprimento atual do nome (0..WEB_SCORE_NAME-1). */
int web_persist_name_length(void);

/* Devolve o codigo do caractere na posicao, ou -1. */
int web_persist_name_char(int index);

/* Limpa o buffer do nome. */
void web_persist_name_clear(void);

/* Devolve e limpa a acao pendente: 1=confirmar, -1=cancelar, 0=nenhuma. */
int web_persist_name_take_action(void);

/* Ativa/desativa o modo de entrada de nome (sinaliza ao JS que deve encaminhar
   teclas para web_persist_name_feed em vez das acoes de jogo). */
void web_persist_name_start(void);
void web_persist_name_stop(void);
int web_persist_name_active(void);

/* ===== API para main.c obter resultado uma única vez ===== */
/* kind: 1=LOAD, 2=SAVE, 3=SCORE_SUBMIT, 4=SCORE_LIST. Preenche *out_player e
   *out_level apenas se outcome==1 e kind==1. Retorna 1 se havia resultado
   pronto e foi consumido, 0 caso contrario. */
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
