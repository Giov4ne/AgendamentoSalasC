#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// CONSTANTES
#define TAM_MAX_STRING 80
#define SENHA_ADMIN "1234"


// ESTRUTURAS
typedef struct{
    int dia;
    int mes;
    int ano;
} Data;

typedef struct{
    int horas;
    int minutos;
} Horario;

typedef struct{
    int id;
    char *nome;
    char *descricao;
    int lotacaoMaxima; 
} Sala;

typedef struct{
    int id;
    int idSala;
    Data data;
    Horario horarioInicio;
    Horario horarioFim;
    int qtdPessoas;
} Reserva;


// FUNÇÕES
void obterDataEHoraAtual(Data *dataAtual, Horario *horarioAtual);
Sala * lerSalas(FILE *arq, char *nomeArq, int *qtdSalas);
Reserva * lerReservas(FILE *arq, char *nomeArq, int *qtdReservas, Data dataAtual, Horario horarioAtual);
void listarSalas(Sala *salas, int qtdSalas);
void listarReservas(Reserva *reservas, int qtdReservas, Sala *salas, int qtdSalas);
void listarReservasDeUmaSala(Reserva *reservas, int qtdReservas, Sala *salas, int qtdSalas);
Reserva * agendarSala(Reserva *reservas, int *qtdReservas, Sala *salas, int qtdSalas, Data dataAtual, Horario horarioAtual);
Reserva * cancelarReserva(Reserva *reservas, int *qtdReservas, Reserva *listaAnteriorDeCancelamentos, int *qtdCancelamentos);
Sala * cadastrarSala(Sala *salas, int *qtdSalas);
Sala * excluirSala(Sala *salas, int *qtdSalas, Reserva **reservas, int *qtdReservas);
void passarSalasParaArquivo(FILE *arq, char *nomeArq, Sala *salas, int qtdSalas);
void passarReservasParaArquivo(FILE *arq, char *nomeArq, Reserva *reservas, int qtdReservas);
void passarCancelamentosParaArquivo(FILE *arq, char *nomeArq, Reserva *cancelamentos, int qtdCancelamentos);
void liberarMemoria(Sala **salas, int qtdSalas, Reserva **reservas, Reserva **cancelamentos);

int main(){
    // LEITURA DE ARQUIVOS E OBTENÇÃO DE DATA E HORA ATUAIS
    Data dataAtual;
    Horario horarioAtual;
    obterDataEHoraAtual(&dataAtual, &horarioAtual);
    FILE *arqSalas, *arqReservas, *arqCancelamentos;
    char nomeArqSalas[] = "salas.txt", nomeArqReservas[] = "reservas.txt", nomeArqCancelamentos[] = "cancelamentos.txt";
    int opcao, qtdSalas, qtdReservas, qtdCancelamentos = 0;
    Sala *salas = lerSalas(arqSalas, nomeArqSalas, &qtdSalas);
    Reserva *reservas = lerReservas(arqReservas, nomeArqReservas, &qtdReservas, dataAtual, horarioAtual), *cancelamentos;
    if(qtdSalas > 0 && salas == NULL || qtdReservas > 0 && reservas == NULL){
        printf("ERRO: Nao foi possivel ler os arquivos");
        liberarMemoria(&salas, qtdSalas, &reservas, &cancelamentos);
        return 1;
    }
    // MENU DE OPÇÕES
    do{
        printf("\n========= MENU =========\n");
        printf("1- Listar salas\n");
        printf("2- Listar todas as reservas\n");
        printf("3- Listar reservas de uma sala\n");
        printf("4- Agendar sala\n");
        printf("5- Cancelar reserva\n");
        printf("6- Cadastrar nova sala (ADMIN)\n");
        printf("7- Excluir sala (ADMIN)\n");
        printf("8- Encerrar o programa\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        switch(opcao){
            case 1: // LISTAR SALAS
                listarSalas(salas, qtdSalas);
            break;
            case 2: // LISTAR TODAS AS RESERVAS
                listarReservas(reservas, qtdReservas, salas, qtdSalas);
            break;
            case 3: // LISTAR RESERVAS DE UMA SALA
                listarSalas(salas, qtdSalas);
                listarReservasDeUmaSala(reservas, qtdReservas, salas, qtdSalas);
            break;
            case 4: // AGENDAR SALA
                reservas = agendarSala(reservas, &qtdReservas, salas, qtdSalas, dataAtual, horarioAtual);
            break;
            case 5: // CANCELAR RESERVA
                listarReservas(reservas, qtdReservas, salas, qtdSalas);
                if(qtdReservas > 0)
                    cancelamentos = cancelarReserva(reservas, &qtdReservas, cancelamentos, &qtdCancelamentos);
                if(qtdReservas > 0 && reservas == NULL){
                    liberarMemoria(&salas, qtdSalas, &reservas, &cancelamentos);
                    return 1;
                }
            break;
            case 6: // CADASTRAR NOVA SALA
                salas = cadastrarSala(salas, &qtdSalas);
            break;
            case 7: // EXCLUIR SALA
                if(qtdSalas > 0)
                    salas = excluirSala(salas, &qtdSalas, &reservas, &qtdReservas);
                if(qtdReservas > 0 && reservas == NULL){
                    liberarMemoria(&salas, qtdSalas, &reservas, &cancelamentos);
                    return 1;
                }
            break;
            case 8: // ENCERRAR PROGRAMA
                listarReservas(reservas, qtdReservas, salas, qtdSalas);
                printf("\nEncerrando...\n");
                passarSalasParaArquivo(arqSalas, nomeArqSalas, salas, qtdSalas);
                passarReservasParaArquivo(arqReservas, nomeArqReservas, reservas, qtdReservas);
                passarCancelamentosParaArquivo(arqCancelamentos, nomeArqCancelamentos, cancelamentos, qtdCancelamentos);
                liberarMemoria(&salas, qtdSalas, &reservas, &cancelamentos);
            break;
            default: // CASO USUÁRIO DIGITE OPÇÃO INEXISTENTE
                printf("Opcao invalida! Informe um numero de 1 a 8.\n");
            break;
        }
    } while(opcao != 8);
    return 0;
}

void obterDataEHoraAtual(Data *dataAtual, Horario *horarioAtual){
    time_t dt = time(NULL), hr;
    time(&hr);
    struct tm tm = *localtime(&dt), *infoTempo = localtime(&hr);
    dataAtual->dia = tm.tm_mday;
    dataAtual->mes = tm.tm_mon + 1;
    dataAtual->ano = tm.tm_year + 1900;
    horarioAtual->horas = infoTempo->tm_hour;
    horarioAtual->minutos = infoTempo->tm_min;
}

Sala * lerSalas(FILE *arq, char *nomeArq, int *qtdSalas){
    Sala *salas;
    if((arq = fopen(nomeArq, "r")) != NULL){
        fscanf(arq, "%d Sala(s) cadastrada(s)\n", qtdSalas);
        if(qtdSalas > 0){
            int tamLista = 0;
            char *lixo = malloc(36 * sizeof(char));
            fscanf(arq, "ID, Nome, Descricao, Lotacao Maxima\n", lixo);
            free(lixo);
            for(int i = 0; i < *qtdSalas; i++){
                Sala s;
                int tamSala, tamNome, tamDescricao;
                s.nome = malloc(TAM_MAX_STRING * sizeof(char));
                s.descricao = malloc(TAM_MAX_STRING * sizeof(char));
                fscanf(arq, "%d, %[^,], %[^,], %d\n", &s.id, s.nome, s.descricao, &s.lotacaoMaxima);
                tamNome = (strlen(s.nome) + 1) * sizeof(char);
                tamDescricao = (strlen(s.descricao) + 1) * sizeof(char);
                s.nome = realloc(s.nome, tamNome);
                s.descricao = realloc(s.descricao, tamDescricao);
                if(s.nome == NULL || s.descricao == NULL){
                    printf("ERRO: Nao foi possivel ler os dados do arquivo.\n");
                    for(int j = 0; j <= i; j++){
                        free(salas[j].nome);
                        free(salas[j].descricao);
                    }
                    free(salas);
                    return NULL;
                }
                tamSala = (2 * sizeof(int)) + tamNome + tamDescricao;
                tamLista += tamSala;
                if(i > 0){
                    salas = realloc(salas, tamLista);
                } else{
                    salas = malloc(tamSala);
                }
                if(salas == NULL)
                    return salas;
                salas[i] = s;
            }
            fclose(arq);
        }
    } else{
        printf("ERRO: Nao foi possivel ler o conteudo do arquivo %s\n", nomeArq);
    }
    return salas;
}

Reserva * lerReservas(FILE *arq, char *nomeArq, int *qtdReservas, Data dataAtual, Horario horarioAtual){
    Reserva *reservas;
    if((arq = fopen(nomeArq, "r")) != NULL){
        fscanf(arq, "%d Reserva(s) cadastrada(s)\n", qtdReservas);
        if(*qtdReservas > 0){
            int tamLista = 0;
            char *lixo = malloc(73 * sizeof(char));
            fscanf(arq, "ID, Id da Sala, Data, Horario Inicio, Horario Fim, Quantidade de Pessoas\n", lixo);
            free(lixo);
            for(int i = 0, c = 0; c < *qtdReservas; i++){
                Reserva r;
                fscanf(arq, "%d, %d, %d/%d/%d, %d:%d, %d:%d, %d\n", &r.id, &r.idSala, &r.data.dia, &r.data.mes, &r.data.ano, &r.horarioInicio.horas, &r.horarioInicio.minutos, &r.horarioFim.horas, &r.horarioFim.minutos, &r.qtdPessoas);
                if(r.data.ano > dataAtual.ano || r.data.ano == dataAtual.ano && r.data.mes > dataAtual.mes || r.data.ano == dataAtual.ano && r.data.mes == dataAtual.mes && r.data.dia > dataAtual.dia || r.data.ano == dataAtual.ano && r.data.mes == dataAtual.mes && r.data.dia == dataAtual.dia && (r.horarioFim.horas > horarioAtual.horas || r.horarioFim.horas == horarioAtual.horas && r.horarioFim.minutos > horarioAtual.minutos)){ // VERIFICA SE A DATA E HORÁRIO DA RESERVA SÃO POSTERIORES À DATA E AO HORARIO ATUAL
                    int tamReserva = 10 * sizeof(int);
                    tamLista += tamReserva;
                    if(c > 0){
                        reservas = realloc(reservas, tamLista);
                    } else{
                        reservas = malloc(tamReserva);
                    }
                    if(reservas == NULL)
                        return reservas;
                    reservas[c++] = r;
                } else{
                    (*qtdReservas)--;
                }
            }
            fclose(arq);
        }
    } else{
        printf("ERRO: Nao foi possivel ler o conteudo do arquivo %s\n", nomeArq);
    }
    return reservas;
}

void listarSalas(Sala *salas, int qtdSalas){
    if(qtdSalas > 0){
        for(int i = 0; i < qtdSalas; i++){
            printf("\n%s\n", salas[i].nome);
            printf("Descricao: %s\n", salas[i].descricao);
            printf("Lotacao maxima: %d pessoas\n", salas[i].lotacaoMaxima);
            printf("Id: %d\n", salas[i].id);
        }
    } else{
        printf("Nao ha salas cadastradas!\n");
    }
}

void listarReservas(Reserva *reservas, int qtdReservas, Sala *salas, int qtdSalas){
    if(qtdReservas > 0){
        for(int i = 0; i < qtdReservas; i++){
            Sala salaCorrespondente;
            for(int j = 0, condicao = 0; j < qtdSalas && condicao == 0; j++){ // ENCONTRAR SALA REFERENTE A RESERVA
                if(reservas[i].idSala == salas[j].id){
                    salaCorrespondente = salas[j];
                    condicao = 1;
                }
            }
            printf("\nReserva %d\n", reservas[i].id);
            printf("%s (ID %d) - %s\n", salaCorrespondente.nome, salaCorrespondente.id, salaCorrespondente.descricao);
            printf("Data: %02d/%02d/%04d\n", reservas[i].data.dia, reservas[i].data.mes, reservas[i].data.ano);
            printf("Horario: %02d:%02d ate %02d:%02d\n", reservas[i].horarioInicio.horas, reservas[i].horarioInicio.minutos, reservas[i].horarioFim.horas, reservas[i].horarioFim.minutos);
            printf("Quantidade de pessoas: %d\n", reservas[i].qtdPessoas);
        }
    } else{
        printf("Nao ha nenhuma reserva no momento!\n");
    }
}

void listarReservasDeUmaSala(Reserva *reservas, int qtdReservas, Sala *salas, int qtdSalas){
    if(qtdReservas > 0){
        Sala salaCorrespondente;
        int idSala, salaEncontrada = 0;
        printf("\n");
        do{
            printf("Digite o ID da sala, ou -1 para cancelar a operacao: ");
            scanf("%d", &idSala);
            if(idSala == -1){
                printf("Operacao cancelada!\n");
            } else{
                for(int i = 0; i < qtdSalas && salaEncontrada == 0; i++){
                    if(salas[i].id == idSala){
                        salaEncontrada = 1;
                        salaCorrespondente = salas[i];
                    }
                }
                if(salaEncontrada == 0){
                    printf("Nao existe nenhuma sala com o ID %d!\n", idSala);
                } else{
                    printf("\nReservas para %s - %s:\n", salaCorrespondente.nome, salaCorrespondente.descricao);
                    int c = 0;
                    for(int i = 0; i < qtdReservas; i++){
                        if(reservas[i].idSala == salaCorrespondente.id){
                            c++;
                            printf("\nReserva %d\n", reservas[i].id);
                            printf("Data: %02d/%02d/%04d\n", reservas[i].data.dia, reservas[i].data.mes, reservas[i].data.ano);
                            printf("Horario: %02d:%02d ate %02d:%02d\n", reservas[i].horarioInicio.horas, reservas[i].horarioInicio.minutos, reservas[i].horarioFim.horas, reservas[i].horarioFim.minutos);
                            printf("Quantidade de pessoas: %d\n", reservas[i].qtdPessoas);
                        }
                    }
                    if(c == 0)
                        printf("Nao ha nenhuma reserva para esta sala!\n");
                }
            }
        } while(salaEncontrada == 0 && idSala != -1);
    } else{
        printf("Nao ha nenhuma reserva no momento!\n");
    }
}

Reserva * agendarSala(Reserva *reservas, int *qtdReservas, Sala *salas, int qtdSalas, Data dataAtual, Horario horarioAtual){
    listarSalas(salas, qtdSalas);
    if(qtdSalas > 0){
        Reserva *novaLista, r, reservaCorrespondente;
        int salaEncontrada = 0, menorDiferenca = 0, trocarDeSala, permissaoParaReservar = 1, tamReserva = 10 * sizeof(int);
        Sala salaCorrespondente, salaComMenorDiferenca;
        r.id = (*qtdReservas > 0) ? reservas[*qtdReservas - 1].id + 1 : 1;
        printf("\n");
        do{ // LEITURA DA SALA
            printf("Digite o ID da sala que deseja agendar, ou -1 para cancelar a operacao: ");
            scanf("%d", &r.idSala);
            if(r.idSala == -1){
                printf("Operacao cancelada!\n");
                return reservas;
            } else{
                for(int i = 0; i < qtdSalas; i++){
                    if(salas[i].id == r.idSala){
                        salaEncontrada = 1;
                        salaCorrespondente = salas[i];
                        salaComMenorDiferenca = salaCorrespondente;
                    }
                }
                if(salaEncontrada == 0)
                    printf("Nao existe nenhuma sala com o ID %d\n", r.idSala);
                else
                    printf("\n%s - %s (capacidade: %d pessoas)\n", salaCorrespondente.nome, salaCorrespondente.descricao, salaCorrespondente.lotacaoMaxima);
            }
        } while(salaEncontrada == 0 && r.idSala != -1);

        do{ // LEITURA DA QUANTIDADE DE PESSOAS PARA RESERVA
            printf("Quantidade de pessoas: ");
            scanf("%d", &r.qtdPessoas);
            if(r.qtdPessoas < 1){
                printf("A reserva deve possuir no minimo uma pessoa!\n");
            } else if(r.qtdPessoas > salaCorrespondente.lotacaoMaxima){
                printf("Numero de pessoas maior que a lotacao maxima da sala, impossivel reservar!\n");
                return reservas;
            } else{
                menorDiferenca = abs(salaCorrespondente.lotacaoMaxima - r.qtdPessoas);
            }
        } while(r.qtdPessoas < 1);

        for(int i = 0; i < qtdSalas; i++){ // VERIFICA SE HÁ UMA SALA COM CAPACIDADE MAIS IDEAL DO QUE A SALA ESCOLHIDA PARA A QUANTIDADE DE PESSOAS DA RESERVA
            if(salas[i].id != salaCorrespondente.id && salas[i].lotacaoMaxima >= r.qtdPessoas){
                int diferenca = abs(salas[i].lotacaoMaxima - r.qtdPessoas);
                if(diferenca < menorDiferenca){
                    menorDiferenca = diferenca;
                    salaComMenorDiferenca = salas[i];
                }        
            }
        }
        if(salaComMenorDiferenca.id != salaCorrespondente.id){
            printf("\nA %s - %s tem uma capacidade mais ideal para a quantidade de pessoas da sua reserva.\n", salaComMenorDiferenca.nome, salaComMenorDiferenca.descricao);
            printf("Deseja reservar a %s ao inves da %s?\n", salaComMenorDiferenca.nome, salaCorrespondente.nome);
            do{
                printf("Digite 1 para 'sim' ou 2 para 'nao': ");
                scanf("%d", &trocarDeSala);
                if(trocarDeSala != 1 && trocarDeSala != 2){
                    printf("As unicas opcoes sao 1 ou 2!\n");
                } else if(trocarDeSala == 1){
                    salaCorrespondente = salaComMenorDiferenca;
                    r.idSala = salaCorrespondente.id;
                    printf("Reserva alterada para a %s - %s\n", salaCorrespondente.nome, salaCorrespondente.descricao);
                }
            } while(trocarDeSala != 1 && trocarDeSala != 2);
        }

        do{ // LEITURA DA DATA DE RESERVA
            printf("\nData da reserva (dd/mm/aaaa): ");
            scanf("%d/%d/%d", &r.data.dia, &r.data.mes, &r.data.ano);
            if(r.data.dia < 1 || r.data.dia > 31 || r.data.mes < 1 || r.data.mes > 12 || r.data.ano < 1){ // VERIFICA SE DATA É VÁLIDA
                printf("Informe uma data valida!\n");
            } else if(dataAtual.ano > r.data.ano || dataAtual.ano == r.data.ano && dataAtual.mes > r.data.mes || dataAtual.ano == r.data.ano && dataAtual.mes == r.data.mes && dataAtual.dia > r.data.dia){ // VERIFICA SE DATA DA RESERVA É IGUAL OU POSTERIOR À DATA ATUAL
                printf("A data da reserva precisa ser igual ou posterior a data atual!");
            }
        } while(r.data.dia < 1 || r.data.dia > 31 || r.data.mes < 1 || r.data.mes > 12 || r.data.ano < 1 || dataAtual.ano > r.data.ano || dataAtual.ano == r.data.ano && dataAtual.mes > r.data.mes || dataAtual.ano == r.data.ano && dataAtual.mes == r.data.mes && dataAtual.dia > r.data.dia);

        do{ // LEITURA DO HORÁRIO DE INÍCIO
            printf("Horario de inicio (hh:mm): ");
            scanf("%d:%d", &r.horarioInicio.horas, &r.horarioInicio.minutos);
            if(r.horarioInicio.horas < 0 || r.horarioInicio.horas > 23 || r.horarioInicio.minutos < 0 || r.horarioInicio.minutos > 59) // VERIFICA SE HORÁRIO É VÁLIDO
                printf("Horario invalido!\n");
            else if(dataAtual.ano == r.data.ano && dataAtual.mes == r.data.mes && dataAtual.dia == r.data.dia && (r.horarioInicio.horas < horarioAtual.horas || r.horarioInicio.horas == horarioAtual.horas && r.horarioInicio.minutos < horarioAtual.minutos)) // VERIFICA SE HORÁRIO DE INÍCIO É IGUAL OU POSTERIOR AO HORÁRIO ATUAL
                printf("O horario de inicio deve ser igual ou posterior ao horario atual!\n");
        } while(r.horarioInicio.horas < 0 || r.horarioInicio.horas > 23 || r.horarioInicio.minutos < 0 || r.horarioInicio.minutos > 59 || dataAtual.ano == r.data.ano && dataAtual.mes == r.data.mes && dataAtual.dia == r.data.dia && (r.horarioInicio.horas < horarioAtual.horas || r.horarioInicio.horas == horarioAtual.horas && r.horarioInicio.minutos < horarioAtual.minutos));

        do{ // LEITURA DO HORÁRIO DE TÉRMINO
            printf("Horario de termino (hh:mm): ");
            scanf("%d:%d", &r.horarioFim.horas, &r.horarioFim.minutos);
            if(r.horarioFim.horas < 0 || r.horarioFim.horas > 23 || r.horarioFim.minutos < 0 || r.horarioFim.minutos > 59){ // VERIFICA SE HORÁRIO É VÁLIDO
                printf("Horario invalido!\n");
            } else if(r.horarioFim.horas < r.horarioInicio.horas || r.horarioFim.horas == r.horarioInicio.horas && r.horarioFim.minutos <= r.horarioInicio.minutos){ // VERIFICA SE HORÁRIO DE TÉRMINO É POSTERIOR AO HORÁRIO DE INÍCIO
                printf("O horario de termino deve ser posterior ao horario de inicio!\n");
            }
        } while(r.horarioFim.horas < 0 || r.horarioFim.horas > 23 || r.horarioFim.minutos < 0 || r.horarioFim.minutos > 59 || r.horarioFim.horas < r.horarioInicio.horas || r.horarioFim.horas == r.horarioInicio.horas && r.horarioFim.minutos <= r.horarioInicio.minutos);

        for(int i = 0; i < *qtdReservas && permissaoParaReservar == 1; i++){ // VERIFICA SE JÁ EXISTE UMA RESERVA PARA A DATA E HORÁRIO ESCOLHIDOS
            if(reservas[i].idSala == salaCorrespondente.id)

                if(reservas[i].data.ano == r.data.ano && reservas[i].data.mes == r.data.mes && reservas[i].data.dia == r.data.dia)

                    if(((r.horarioInicio.horas > reservas[i].horarioInicio.horas || r.horarioInicio.horas == reservas[i].horarioInicio.horas && r.horarioInicio.minutos >= reservas[i].horarioInicio.minutos) && (r.horarioInicio.horas < reservas[i].horarioFim.horas || r.horarioInicio.horas == reservas[i].horarioFim.horas && r.horarioInicio.minutos < reservas[i].horarioFim.minutos)) || ((r.horarioFim.horas > reservas[i].horarioInicio.horas || r.horarioFim.horas == reservas[i].horarioInicio.horas && r.horarioFim.minutos > reservas[i].horarioInicio.minutos) && (r.horarioFim.horas < reservas[i].horarioFim.horas || r.horarioFim.horas == reservas[i].horarioFim.horas && r.horarioFim.minutos <= reservas[i].horarioFim.minutos)) || ((r.horarioInicio.horas < reservas[i].horarioInicio.horas || r.horarioInicio.horas == reservas[i].horarioInicio.horas && r.horarioInicio.minutos <= reservas[i].horarioInicio.minutos) && (r.horarioFim.horas > reservas[i].horarioFim.horas || r.horarioFim.horas == reservas[i].horarioFim.horas && r.horarioFim.minutos >= reservas[i].horarioFim.minutos))){
 
                        permissaoParaReservar = 0;
                        reservaCorrespondente = reservas[i];

                    }
        }

        if(permissaoParaReservar == 0){ // CASO JÁ HAJA RESERVA PARA A DATA E HORÁRIO ESCOLHIDOS
            printf("Impossivel reservar para esse dia e horario.\n");
            printf("%s ja reservada para o dia %02d/%02d/%04d das %02d:%02d as %02d:%02d!\n", salaCorrespondente.nome, reservaCorrespondente.data.dia, reservaCorrespondente.data.mes, reservaCorrespondente.data.ano, reservaCorrespondente.horarioInicio.horas, reservaCorrespondente.horarioInicio.minutos, reservaCorrespondente.horarioFim.horas, reservaCorrespondente.horarioFim.minutos);
            return reservas;
        }

        if(*qtdReservas > 0)
            novaLista = realloc(reservas, (*qtdReservas + 1) * tamReserva);
        else
            novaLista = malloc(tamReserva);

        if(novaLista == NULL){
            printf("ERRO: Nao foi possivel alocar a reserva na memoria!\n");
            return reservas;
        }

        novaLista[(*qtdReservas)++] = r;
        printf("Agendamento concluido com sucesso!\n");

        return novaLista;
    }
}

Reserva * cancelarReserva(Reserva *reservas, int *qtdReservas, Reserva *listaAnteriorDeCancelamentos, int *qtdCancelamentos){
    int idCancel, reservaEncontrada = 0, tamReserva = 10 * sizeof(int);
    Reserva r, *cancelamentos;
    printf("\n");
    do{
        printf("Digite o ID da reserva a ser cancelada, ou -1 para cancelar a operacao: ");
        scanf("%d", &idCancel);
        if(idCancel == -1){
            printf("Operacao cancelada!\n");
            return listaAnteriorDeCancelamentos;
        }
        for(int i = 0; i < *qtdReservas && reservaEncontrada == 0; i++){
            if(idCancel == reservas[i].id){
                reservaEncontrada = 1;
                r = reservas[i];
                for(int j = i; j < *qtdReservas - 1; j++){
                    reservas[j] = reservas[j + 1];
                }
                reservas = realloc(reservas, (--(*qtdReservas)) * tamReserva);
                if(*qtdReservas > 0 && reservas == NULL){
                    printf("ERRO: Nao foi possivel realocar memoria!\n");
                    return NULL;
                }
            }
        }
        if(reservaEncontrada == 1){
            if(*qtdCancelamentos > 0){
                cancelamentos = realloc(listaAnteriorDeCancelamentos, (*qtdCancelamentos + 1) * tamReserva);
            } else{
                cancelamentos = malloc(tamReserva);
            }
            if(cancelamentos == NULL){
                printf("ERRO: Operacao cancelada, nao foi possivel alocar memoria!\n");
                return listaAnteriorDeCancelamentos;
            }
            cancelamentos[(*qtdCancelamentos)++] = r;
            printf("Reserva cancelada com sucesso!\n");
        } else{
            printf("Nao ha nenhuma reserva com este ID!\n");
        }
    } while(reservaEncontrada == 0);
    return cancelamentos;
}

Sala * cadastrarSala(Sala *salas, int *qtdSalas){
    char *senha = malloc(TAM_MAX_STRING);
    printf("Senha de administrador: ");
    scanf("%s", senha);
    if(strcmp(SENHA_ADMIN, senha) == 0){ // A FUNÇÃO SÓ PERMITE CADASTRAR SALA SE O USUÁRIO INFORMAR A SENHA DE ADMINISTRADOR
        free(senha);
        Sala *novaLista, s;
        int tamSala, tamNome, tamDescricao, tamListaDeSalas = 0;
        s.id = (*qtdSalas > 0) ? salas[*qtdSalas - 1].id + 1 : 1; 

        s.nome = malloc(TAM_MAX_STRING);
        printf("Nome da sala: ");
        getchar(); // LIMPAR BUFFER DO TECLADO
        fgets(s.nome, TAM_MAX_STRING, stdin);
        tamNome = strlen(s.nome) * sizeof(char);
        s.nome[tamNome - 1] = '\0';
        if(tamNome < TAM_MAX_STRING)
            s.nome = realloc(s.nome, tamNome);
        if(s.nome == NULL){
            printf("ERRO: Nao foi possivel alocar sala na memoria!\n");
            return salas;
        }
            
        s.descricao = malloc(TAM_MAX_STRING);
        printf("Descricao: ");
        fgets(s.descricao, TAM_MAX_STRING, stdin);
        tamDescricao = strlen(s.descricao) * sizeof(char);
        s.descricao[tamDescricao - 1] = '\0';
        if(tamDescricao < TAM_MAX_STRING)
            s.descricao = realloc(s.descricao, tamDescricao);
        if(s.descricao == NULL){
            printf("ERRO: Nao foi possivel alocar sala na memoria!\n");
            return salas;
        }

        do{
            printf("Lotacao maxima de pessoas: ");
            scanf("%d", &s.lotacaoMaxima);
            if(s.lotacaoMaxima < 1)
                printf("Valor invalido! A capacidade deve ser de no minimo uma pessoa\n");
        } while(s.lotacaoMaxima < 1);

        tamSala = 2 * sizeof(int) + tamNome + tamDescricao;

        if(*qtdSalas > 0){
            for(int i = 0; i < *qtdSalas; i++)
                tamListaDeSalas += 2 * sizeof(int) + (strlen(salas[i].nome) + strlen(salas[i].descricao) + 2) * sizeof(char);
            novaLista = realloc(salas, tamListaDeSalas + tamSala);
        } else{
            novaLista = malloc(tamSala);
        }

        if(novaLista == NULL){
            printf("ERRO: Nao foi possivel alocar a lista de salas na memoria!\n");
            return salas;
        }

        novaLista[(*qtdSalas)++] = s;
        printf("%s cadastrada com sucesso!\n", s.nome);
        return novaLista;

    } else{
        printf("Senha incorreta. Operacao cancelada!\n");
        free(senha);
        return salas;
    }
}

Sala * excluirSala(Sala *salas, int *qtdSalas, Reserva **reservas, int *qtdReservas){
    char *senha = malloc(TAM_MAX_STRING);
    printf("Senha de administrador: ");
    scanf("%s", senha);
    if(strcmp(SENHA_ADMIN, senha) == 0){ // A FUNÇÃO SÓ PERMITE EXCLUIR SALA SE O USUÁRIO INFORMAR A SENHA DE ADMINISTRADOR
        listarSalas(salas, *qtdSalas);
        printf("\n");
        free(senha);
        Sala *novaLista, salaASerExcluida;
        int idExclusao, salaEncontrada = 0, tamSalaASerExcluida, tamListaDeSalas = 0, tamReserva = 10 * sizeof(int), tamListaDeReservas = *qtdReservas * tamReserva, contadorDeReservasExcluidas = 0;
        do{
            printf("Digite o ID da sala a ser excluida, ou -1 para cancelar a operacao: ");
            scanf("%d", &idExclusao);
            if(idExclusao == -1){
                printf("Operacao cancelada!\n");
                return salas;
            } 
            for(int i = 0; i < *qtdSalas; i++){
                tamListaDeSalas += 2 * sizeof(int) + (strlen(salas[i].nome) + strlen(salas[i].descricao) + 2) * sizeof(char);
                if(idExclusao == salas[i].id){
                    salaEncontrada = 1;
                    salaASerExcluida = salas[i];
                    for(int j = i; j < *qtdSalas - 1; j++){
                        salas[j] = salas[j + 1];
                    }
                }
            }
            if(salaEncontrada == 1){
                tamSalaASerExcluida = 2 * sizeof(int) + (strlen(salaASerExcluida.nome) + strlen(salaASerExcluida.descricao) + 2) * sizeof(char);
                novaLista = realloc(salas, tamListaDeSalas - tamSalaASerExcluida);
                if(*qtdSalas > 0 && novaLista == NULL){
                    printf("ERRO: Nao foi possivel realocar memoria!\n");
                    return salas;
                }
                printf("\n%s excluida com sucesso!\n", salaASerExcluida.nome);
                (*qtdSalas)--;
            } else{
                printf("Nao existe nenhuma sala com o ID %d!\n", idExclusao);
            }
        } while(salaEncontrada == 0);

        // EXCLUSÃO DE RESERVAS RELACIONADAS À SALA EXCLUÍDA
        for(int i = 0; i < *qtdReservas; i++){ 
            if((*reservas)[i].idSala == idExclusao){
                tamListaDeReservas -= tamReserva;
                contadorDeReservasExcluidas++;
                for(int j = i; j < *qtdReservas - 1; j++)
                    (*reservas)[j] = (*reservas)[j + 1];
            }
        }
        
        if(*qtdReservas > 0 && *reservas == NULL){
            printf("ERRO: Nao foi possivel realocar memoria.\n");
        } else if(contadorDeReservasExcluidas > 0){
            (*qtdReservas) -= contadorDeReservasExcluidas;
            *reservas = realloc(*reservas, tamListaDeReservas);
            printf("%d reserva(s) da %s foi/foram excluida(s)!\n", contadorDeReservasExcluidas, salaASerExcluida.nome);
        }

        return novaLista;

    } else{
        printf("Senha incorreta. Operacao cancelada!\n");
        free(senha);
        return salas;
    }    
}

void passarSalasParaArquivo(FILE *arq, char *nomeArq, Sala *salas, int qtdSalas){
    if((arq = fopen(nomeArq, "w")) != NULL){
        fprintf(arq, "%d Sala(s) cadastrada(s)\n", qtdSalas);
        fprintf(arq, "ID, Nome, Descricao, Lotacao Maxima\n");
        for(int i = 0; i < qtdSalas; i++){
            fprintf(arq, "%d, %s, %s, %d\n", salas[i].id, salas[i].nome, salas[i].descricao, salas[i].lotacaoMaxima);
        }
        fclose(arq);
    } else{
        printf("ERRO: Nao foi possivel abrir o arquivo %s\n", nomeArq);
    }
}

void passarReservasParaArquivo(FILE *arq, char *nomeArq, Reserva *reservas, int qtdReservas){
    if((arq = fopen(nomeArq, "w")) != NULL){
        fprintf(arq, "%d Reserva(s) cadastrada(s)\n", qtdReservas);
        fprintf(arq, "ID, Id da Sala, Data, Horario Inicio, Horario Fim, Quantidade de Pessoas\n");
        for(int i = 0; i < qtdReservas; i++){
            fprintf(arq, "%d, %d, %d/%d/%d, %d:%d, %d:%d, %d\n", reservas[i].id, reservas[i].idSala, reservas[i].data.dia, reservas[i].data.mes, reservas[i].data.ano, reservas[i].horarioInicio.horas, reservas[i].horarioInicio.minutos, reservas[i].horarioFim.horas, reservas[i].horarioFim.minutos, reservas[i].qtdPessoas);
        }
        fclose(arq);
    } else{
        printf("ERRO: Nao foi possivel abrir o arquivo %s\n", nomeArq);
    }
}

void passarCancelamentosParaArquivo(FILE *arq, char *nomeArq, Reserva *cancelamentos, int qtdCancelamentos){
    if(qtdCancelamentos > 0){
        if((arq = fopen(nomeArq, "a")) != NULL){
            for(int i = 0; i < qtdCancelamentos; i++){
                //ID, Id da Sala, Data, Horario Inicio, Horario Fim, Quantidade de Pessoas
                fprintf(arq, "%d, %d, %d/%d/%d, %d:%d, %d:%d, %d\n", cancelamentos[i].id, cancelamentos[i].idSala, cancelamentos[i].data.dia, cancelamentos[i].data.mes, cancelamentos[i].data.ano, cancelamentos[i].horarioInicio.horas, cancelamentos[i].horarioInicio.minutos, cancelamentos[i].horarioFim.horas, cancelamentos[i].horarioFim.minutos, cancelamentos[i].qtdPessoas);
            }
            fclose(arq);
        } else{
            printf("ERRO: Nao foi possivel abrir o arquivo %s\n", nomeArq);
        }
    }
}

void liberarMemoria(Sala **salas, int qtdSalas, Reserva **reservas, Reserva **cancelamentos){
    if(*salas != NULL){
        for(int i = 0; i < qtdSalas; i++){
            if((*salas)[i].nome != NULL){
                free((*salas)[i].nome);
                (*salas)[i].nome = NULL;
            }
            if((*salas)[i].descricao != NULL){
                free((*salas)[i].descricao);
                (*salas)[i].descricao = NULL;
            }
        }
        free(*salas);
        *salas = NULL;
    }
    if(*reservas != NULL){
        free(*reservas);
        *reservas = NULL;
    }
    if(*cancelamentos != NULL){
        free(*cancelamentos);
        *cancelamentos = NULL;        
    }
}