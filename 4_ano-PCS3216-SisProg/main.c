    ///----------------------------------------------------------------------------------------------------------///
///----------------------------------------------------------------------------------------------------------///
///----------------------------------------------------------------------------------------------------------///
///                                                                                                          ///
///   PCS3216 - Sistemas de Programação - Projeto 2019                                                       ///
///   Autor: Rafael Seiji Uezu Higa                                                                          ///
///   NUSP:9836878                                                                                           ///
///   Prof. João José Neto                                                                                   ///
///                                                                                                          ///
///----------------------------------------------------------------------------------------------------------///
///----------------------------------------------------------------------------------------------------------///
///----------------------------------------------------------------------------------------------------------///

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

//Structs utilizados no projeto------------------------------------------------------------------

struct instructionEmMnemonico {
    char *label;
    char *mnemonico;
    char *operando1;
    char *operando2;
    char *operando3;
};

struct instructionEmHexadec {
    int *mnemonico;
    int *operando1;
    int *operando2;
    int *operando3;
};

struct allInstructions {
    struct instructionEmHexadec instrucaoHexadecimal[500];
    struct instructionEmMnemonico instrucaoMnemonico[500];
    int inicio;     //inicio do codigo, como um endereço de memoria
    int fim;        //fim do codigo, comouma posição de array
};

struct estadoMaquina {
    int memory[65535];
    int R1,R2,R3,R4,R5,R6,R7,R8;
    int PC,SP,FP,RA;
    int enable;
    int isTrace;
};

struct evento {
    int instrucao[4];
};

struct listaLigada {
    struct evento instruEvento[500];
};

struct labelTable {
    char* label;
    int labelAddress;
};

//Funções utilizadas no projeto------------------------------------------------------------------

int fitHexadec(int valor,int esquerdaDireita) {   //esquerdaDireita define se retorna os dois algarismos hexadec mais ou menos significativos
    int resultado;                                //esquerdaDireita = 1 para menos significativos;
                                                  //esquerdaDireita = 2 para mais significativos

    resultado = valor >> 8;

    if(esquerdaDireita == 1) {
        resultado = resultado << 8;
        resultado = valor - resultado;
    }

return resultado;
}

void cleanMemory(int memory[65535]) {
    int i;

    for(i = 0; i < 65535; i++)
        memory[i] = 0;
}

int resolverRegistrador(char *registrador, struct labelTable mapeamentoLabel[500],int labelQtde) {    //transforma referencia de operando em codigo do end/reg referenciado
    int codigo,i;

    if(strcmp(registrador,"R1") == 0) {
        codigo = 0x01;

    } else if(strcmp(registrador,"R2") == 0) {
        codigo = 0x02;
    } else if(strcmp(registrador,"R3") == 0) {
        codigo = 0x03;
    } else if(strcmp(registrador,"R4") == 0) {
        codigo = 0x04;
    } else if(strcmp(registrador,"R5") == 0) {
        codigo = 0x05;
    } else if(strcmp(registrador,"R6") == 0) {
        codigo = 0x06;
    } else if(strcmp(registrador,"R7") == 0) {
        codigo = 0x07;
    } else if(strcmp(registrador,"R8") == 0) {
        codigo = 0x08;
    } else {
        for(i = 0; i < labelQtde; i++) {
            if(strcmp(registrador, mapeamentoLabel[i].label) == 0) {
                codigo = mapeamentoLabel[i].labelAddress;
            }
        }
    }


return codigo;
}

int lerPrograma(struct instructionEmMnemonico mnemonicoInstru[500], char* nomeDoArquivo) {   //abre o arquivo.txt e carrega o programa nele em um vetor, escrito na forma de mnemonicos ainda
    FILE* arquivo;
    int i = 0;
    char line[90];
    const char* tempNomeFile = arquivo;

    arquivo = fopen(nomeDoArquivo, "r");

        if(arquivo == NULL) {
            printf("Arquivo nao pode ser aberto\n");
        }

        while(!feof(arquivo)) {
            fgets(line, 90, arquivo);
            if(line[0] == '\n') {
                i -= 1;
            } else if(line[0] == ' ' || line[0] == '\t') {   //no label
                mnemonicoInstru[i].label = NULL;

                mnemonicoInstru[i].mnemonico = strdup(strtok(line,", \n\t"));

                mnemonicoInstru[i].operando1 = strdup(strtok(NULL,", \n\t"));

                mnemonicoInstru[i].operando2 = strdup(strtok(NULL,", \n\t"));

                mnemonicoInstru[i].operando3 = strdup(strtok(NULL,", \n\t"));

            } else {
                mnemonicoInstru[i].label = strdup(strtok(line,", \n\t"));

                mnemonicoInstru[i].mnemonico = strdup(strtok(NULL,", \n\t"));

                mnemonicoInstru[i].operando1 = strdup(strtok(NULL,", \n\t"));

                mnemonicoInstru[i].operando2 = strdup(strtok(NULL,", \n\t"));

                mnemonicoInstru[i].operando3 = strdup(strtok(NULL,", \n\t"));

            }
            i++;
        }
    fclose(arquivo);

return i;    //antes tava retornando i+1. Pq???
}

struct allInstructions converterParaHexadecimal(struct allInstructions convertidoHexadecimal, int tamanho) {

    int i = 0,j = 0, qtdeLabels;
    int findInicio = 0;
    int aux,aux2;
    struct labelTable mapeamentoLabel[500];

    while(findInicio == 0) {
        if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL) && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"@") == 0) {
            convertidoHexadecimal.inicio = atoi(convertidoHexadecimal.instrucaoMnemonico[i].operando1);   //operando 1 tem o endereço de inicio pra gravar na memoria
            findInicio = 1;
        }
        i++;
    }

    //Assumir que @ NÃO TEM LABEL!!! o codigo tem q começar com o @ e n ter NADA antes!!!
    for(i = 0; i < tamanho; i++) {  //Primeiro passo do montador
        if(convertidoHexadecimal.instrucaoMnemonico[i].label != NULL) {
            mapeamentoLabel[j].label = convertidoHexadecimal.instrucaoMnemonico[i].label;
            mapeamentoLabel[j].labelAddress = (i - 1) * 4 + convertidoHexadecimal.inicio;
            j++;
        }
    }

    qtdeLabels = j; //+1 pra ficar o num real, e n o num de indice do vetor. qqr coisa somar 1 explicitamente pra usar em loops

    for(i = 0; i < tamanho; i++) {
        //----------------------------------------------------------------------------------------------------------------------
        //Instruções de Acesso à Memória
        //----------------------------------------------------------------------------------------------------------------------
        if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
           && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"LW") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x01;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = 0;

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"SW") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x02;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = 0;

        //----------------------------------------------------------------------------------------------------------------------
        //Loops, controle de fluxo, desvios e chamadas de procedimento
        //----------------------------------------------------------------------------------------------------------------------
        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"BEQ") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x11;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando3,mapeamentoLabel,qtdeLabels);

            aux = convertidoHexadecimal.instrucaoHexadecimal[i].operando2;
            aux = aux << 4;
            aux2 = convertidoHexadecimal.instrucaoHexadecimal[i].operando3;

            aux += aux2;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = aux;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),2);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),1);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"BZR") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x12;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = convertidoHexadecimal.instrucaoHexadecimal[i].operando2;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),2);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),1);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"BNE") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x13;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando3,mapeamentoLabel,qtdeLabels);

            aux = convertidoHexadecimal.instrucaoHexadecimal[i].operando2;
            aux = aux << 4;
            aux2 = convertidoHexadecimal.instrucaoHexadecimal[i].operando3;

            aux += aux2;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = aux;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),2);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),1);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"J") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x14;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),2);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),1);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"JAL") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x15;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),2);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = fitHexadec(resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels),1);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = 0;

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"RTS") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x16;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = 0;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = 0;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = 0;

        //----------------------------------------------------------------------------------------------------------------------
        //Operações Aritméticas
        //----------------------------------------------------------------------------------------------------------------------
        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"ADD") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x21;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);

            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);

            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando3,mapeamentoLabel,qtdeLabels);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"ADDI") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x22;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = atoi(convertidoHexadecimal.instrucaoMnemonico[i].operando3);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"SUB") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x23;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando3,mapeamentoLabel,qtdeLabels);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"MUL") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x24;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando3,mapeamentoLabel,qtdeLabels);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"DIV") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x25;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando3,mapeamentoLabel,qtdeLabels);

        //----------------------------------------------------------------------------------------------------------------------
        //Operações Lógicas
        //----------------------------------------------------------------------------------------------------------------------
        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"AND") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x31;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando3,mapeamentoLabel,qtdeLabels);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"OR") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x32;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando3,mapeamentoLabel,qtdeLabels);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"XOR") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x33;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando3,mapeamentoLabel,qtdeLabels);

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"NOT") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x34;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = 0;

        //----------------------------------------------------------------------------------------------------------------------
        //Outros
        //----------------------------------------------------------------------------------------------------------------------
        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"HM") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x41;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = 0;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = 0;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = 0;

        //----------------------------------------------------------------------------------------------------------------------
        //Pseudoinstruções
        //----------------------------------------------------------------------------------------------------------------------
        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"MOV") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x51;

            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando1,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = resolverRegistrador(convertidoHexadecimal.instrucaoMnemonico[i].operando2,mapeamentoLabel,qtdeLabels);
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = 0;

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"@") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x52;
            //convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = 0;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = 0;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = 0;
            convertidoHexadecimal.inicio = atoi(convertidoHexadecimal.instrucaoMnemonico[i].operando1);   //operando 1 tem o endereço de inicio pra gravar na memoria

        } else if((convertidoHexadecimal.instrucaoMnemonico[i].mnemonico != NULL)
                  && strcmp(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico,"#") == 0) {
            convertidoHexadecimal.instrucaoHexadecimal[i].mnemonico = 0x53;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando1 = 0;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando2 = 0;
            convertidoHexadecimal.instrucaoHexadecimal[i].operando3 = 0;

        } else if(convertidoHexadecimal.instrucaoMnemonico[i].mnemonico == NULL) {
            //Não faz nada!
        } else {
            printf("Erro\n");
        }
    }

return convertidoHexadecimal;
}

void trueLoader(int memory[65535],int endInicial,struct allInstructions programa,int tamanho) {  //carrega o programa na memoria
    int i,j = endInicial;                                                                        //Resolve as pseudoinstruções, trocando-as por instruções "verdadeiras"
    int initialProgramCounter;

    for(i = 0; i < tamanho; i++) {
        if(programa.instrucaoHexadecimal[i].mnemonico == 0x53) {   //#
            memory[j] = 0x41;                                      //Pseudoinstruçao sendo trocada por instrução "verdadeira"
            j++;
            break;
        } else if(programa.instrucaoHexadecimal[i].mnemonico == 0x52) {
            //memory[j] = programa.instrucaoHexadecimal[i].mnemonico;   //@
            //j++;
        } else if(programa.instrucaoHexadecimal[i].mnemonico == 0x51) { //MOV
            memory[j] = 0x23;                                          j++;     //Clear com instrução SUB
            memory[j] = programa.instrucaoHexadecimal[i].operando1;    j++;
            memory[j] = programa.instrucaoHexadecimal[i].operando1;    j++;
            memory[j] = programa.instrucaoHexadecimal[i].operando1;    j++;
            memory[j] = 0x21;                                          j++;     //ADD
            memory[j] = programa.instrucaoHexadecimal[i].operando1;    j++;
            memory[j] = programa.instrucaoHexadecimal[i].operando1;    j++;
            memory[j] = programa.instrucaoHexadecimal[i].operando2;    j++;

        } else {
            memory[j] = programa.instrucaoHexadecimal[i].mnemonico;   j++;
            memory[j] = programa.instrucaoHexadecimal[i].operando1;   j++;
            memory[j] = programa.instrucaoHexadecimal[i].operando2;   j++;
            memory[j] = programa.instrucaoHexadecimal[i].operando3;   j++;
        }
    }
}

int returnRegistradorValor(int operando, struct estadoMaquina machine) {   //retorna valor de operando
    int variavel;

    switch(operando) {
        case 0x01:
            variavel = machine.R1;
            break;
        case 0x02:
            variavel = machine.R2;
            break;
        case 0x03:
            variavel = machine.R3;
            break;
        case 0x04:
            variavel = machine.R4;
            break;
        case 0x05:
            variavel = machine.R5;
            break;
        case 0x06:
            variavel = machine.R6;
            break;
        case 0x07:
            variavel = machine.R7;
            break;
        case 0x08:
            variavel = machine.R8;
            break;
    }

return variavel;
}

struct estadoMaquina setValorOperacao(int operando,int operandoValor, struct estadoMaquina machine) {               //atribui ao opearando o seu novo valor

    switch(operando) {
        case 0x01:
            machine.R1 = operandoValor;
            break;
        case 0x02:
            machine.R2 = operandoValor;
            break;
        case 0x03:
            machine.R3 = operandoValor;
            break;
        case 0x04:
            machine.R4 = operandoValor;
            break;
        case 0x05:
            machine.R5 = operandoValor;
            break;
        case 0x06:
            machine.R6 = operandoValor;
            break;
        case 0x07:
            machine.R7 = operandoValor;
            break;
        case 0x08:
            machine.R8 = operandoValor;
            break;
    }

return machine;
};

int blocoOperacoesAritmeticas(int valor1,int valor2,int operation) {
    int resultado;
    if(operation == 1)
        resultado = valor1 + valor2;
    else if(operation == 2)
        resultado = valor1 - valor2;
    else if(operation == 3)
        resultado = valor1 * valor2;
    else if(operation == 4)
        resultado = valor1 / valor2;

return resultado;
}

int blocoOperacoesLogicas(int valor1, int valor2,int operation) {
    int result;
    if(operation == 1) {  //AND
        result = valor1&valor2;
    } else if(operation == 2) { //OR
        result = valor1|valor2;
    } else if(operation == 3) { //XOR
        result = valor1^valor2;
    } else if(operation == 4) { //NOT
        result = ~valor1;
    }

return result;
}

struct estadoMaquina selectReacoes(struct estadoMaquina machine, struct evento eventNow) {
    int operando1;
    int operando2;
    int operando3;
    int *ponteiro;

    if(eventNow.instrucao[0] == 0x01) {  //LW
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine); //operando 1 = reg/ operando 2 = end.
        machine = setValorOperacao(eventNow.instrucao[1],machine.memory[operando2],machine);

    } else if(eventNow.instrucao[0] == 0x02) {
        operando1 = returnRegistradorValor(eventNow.instrucao[1],machine); //operando 1 = end. de store
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine); //operando 2 = valor q se quer gurdar na memoria
        machine.memory[operando1] = operando2;

    } else if(eventNow.instrucao[0] == 0x11) {  //BEQ
        operando2 = eventNow.instrucao[3];
        operando3 = eventNow.instrucao[3];

        operando2 = operando2 >> 4;
        operando2 = operando2 << 4;

        operando3 = operando3 - operando2;

        operando2 = operando2 >> 4;

        if(operando2 == operando3) {
            eventNow.instrucao[1] = eventNow.instrucao[1] << 8;
            eventNow.instrucao[1] += eventNow.instrucao[2];
            machine.PC = eventNow.instrucao[1];
        }

    } else if(eventNow.instrucao[0] == 0x12) {  //BZR
        operando2 = returnRegistradorValor(eventNow.instrucao[3],machine);
        if(operando2 == 0) {
            eventNow.instrucao[1] = eventNow.instrucao[1] << 8;
            eventNow.instrucao[1] += eventNow.instrucao[2];
            machine.PC = eventNow.instrucao[1];
        }

    } else if(eventNow.instrucao[0] == 0x13) {  //BNE
        operando2 = eventNow.instrucao[3];
        operando3 = eventNow.instrucao[3];

        operando2 = operando2 >> 4;
        operando2 = operando2 << 4;

        operando3 = operando3 - operando2;

        operando2 = operando2 >> 4;

        if(operando2 != operando3) {
            eventNow.instrucao[1] = eventNow.instrucao[1] << 8;
            eventNow.instrucao[1] += eventNow.instrucao[2];
            machine.PC = eventNow.instrucao[1];
        }

    } else if(eventNow.instrucao[0] == 0x14) {  //J
        eventNow.instrucao[1] = eventNow.instrucao[1] << 8;
        eventNow.instrucao[1] += eventNow.instrucao[2];
        machine.PC = eventNow.instrucao[1];

    } else if(eventNow.instrucao[0] == 0x15) { //JAL
        machine.RA = machine.PC + 4;  //+4 para pegar next instruction
        eventNow.instrucao[1] = eventNow.instrucao[1] << 8;
        eventNow.instrucao[1] += eventNow.instrucao[2];
        machine.PC = eventNow.instrucao[1];

    } else if(eventNow.instrucao[0] == 0x16) { //RTS
        machine.PC = machine.RA;   //obs: +4 já incrementado no RA

    } else if(eventNow.instrucao[0] == 0x21) { //ADD
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine);
        operando3 = returnRegistradorValor(eventNow.instrucao[3],machine);
        operando2 = blocoOperacoesAritmeticas(operando2,operando3,1);

        machine = setValorOperacao(eventNow.instrucao[1],operando2,machine);

    } else if(eventNow.instrucao[0] == 0x22) { //ADDI
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine);
        operando3 = eventNow.instrucao[3];
        operando2 += operando3;
        machine = setValorOperacao(eventNow.instrucao[1],operando2,machine);

    } else if(eventNow.instrucao[0] == 0x23) { //SUB
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine);
        operando3 = returnRegistradorValor(eventNow.instrucao[3],machine);
        operando2 = blocoOperacoesAritmeticas(operando2,operando3,2);
        machine = setValorOperacao(eventNow.instrucao[1],operando2,machine);

    } else if(eventNow.instrucao[0] == 0x24) { //MUL
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine);
        operando3 = returnRegistradorValor(eventNow.instrucao[3],machine);
        operando2 = blocoOperacoesAritmeticas(operando2,operando3,3);
        machine = setValorOperacao(eventNow.instrucao[1],operando2,machine);

    } else if(eventNow.instrucao[0] == 0x25) { //DIV
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine);
        operando3 = returnRegistradorValor(eventNow.instrucao[3],machine);
        operando2 = blocoOperacoesAritmeticas(operando2,operando3,4);
        machine = setValorOperacao(eventNow.instrucao[1],operando2,machine);

    } else if(eventNow.instrucao[0] == 0x31) { //AND
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine);
        operando3 = returnRegistradorValor(eventNow.instrucao[3],machine);
        operando2 = blocoOperacoesLogicas(operando2,operando3,1);
        machine = setValorOperacao(eventNow.instrucao[1],operando2,machine);

    } else if(eventNow.instrucao[0] == 0x32) { //OR
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine);
        operando3 = returnRegistradorValor(eventNow.instrucao[3],machine);
        operando2 = blocoOperacoesLogicas(operando2,operando3,2);
        machine = setValorOperacao(eventNow.instrucao[1],operando2,machine);

    } else if(eventNow.instrucao[0] == 0x33) { //XOR
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine);
        operando3 = returnRegistradorValor(eventNow.instrucao[3],machine);
        operando2 = blocoOperacoesLogicas(operando2,operando3,3);
        machine = setValorOperacao(eventNow.instrucao[1],operando2,machine);

    } else if(eventNow.instrucao[0] == 0x34) { //NOT
        operando2 = returnRegistradorValor(eventNow.instrucao[2],machine);
        operando2 = blocoOperacoesLogicas(operando2,0,4);
        machine = setValorOperacao(eventNow.instrucao[1],operando2,machine);

    } else if(eventNow.instrucao[0] == 0x41) { //HM - Halt Machine
        machine.enable = 0;
    } else if(eventNow.instrucao[0] == 0x61) { //MD
        int startPrintAddress;
        int counter1,counter2;
        printf("\n------------------------------------------------------------------------------------------\n");
        printf("PC= %04x     SP= %04x     FP= %04x     RA= %04x\n",machine.PC,machine.SP,machine.FP,machine.RA);
        printf("R1= %04x     R2= %04x     R3= %04x     R4= %04x\n",machine.R1,machine.R2,machine.R3,machine.R4);
        printf("R5= %04x     R6= %04x     R7= %04x     R8= %04x\n",machine.R5,machine.R6,machine.R7,machine.R8);
        startPrintAddress = eventNow.instrucao[1];

        for(counter1 = 0; counter1 < 10; counter1++) {
            for(counter2 = 0; counter2 < 16; counter2++) {
                printf("%02x  ",machine.memory[startPrintAddress]);
                startPrintAddress++;
            }

            printf("\n");
        }
    } else if(eventNow.instrucao[0] == 0x62) { //MM - Memory Modify
        machine.memory[eventNow.instrucao[1]] = eventNow.instrucao[2];

    } else if(eventNow.instrucao[0] == 0x63) {  //HE
        printf("Catálogo de instruções disponíveis\n");
        printf("Eventos artificiais-----------------------------------------\n");
        printf("HE: Help\n->Sintaxe: HE\n\n");
        printf("MD: Memory display. \n->Sintaxe: MD <endereço>\n\n");
        printf("MM: Memory Modify. \n->Sintaxe: MM <endereço> <conteúdo>\n\n");
        printf("TRC: Trace. Esse comando tanto ativa como desativa o modo trace\n->Sintaxe: TRC\n\n");
        printf("Pseudoinstruções--------------------------------------------\n");
        printf("@: Início do programa. \n->Sintaxe: @ <endereço de início do programa>\n\n");
        printf("#: Final Físico do Programa. \n->Sintaxe: #\n\n");
        printf("MOV: MOVE. \n->Sintaxe: MOV <Registrador 1> <Registrador 2>, em que registrador 1 é o destino e registrador 2 é  valor a ser movido.\n\n");


     } else if(eventNow.instrucao[0] == 0x64) { //TRC
        if(machine.isTrace == 0) {             //TRC deve, durante a execução do programa, ficar inserindo na lista de eventos eventos tipo MD
            machine.isTrace = 1;
            printf("Modo Trace Ativado!\n");
        } else if(machine.isTrace == 1) {
            machine.isTrace = 0;
            printf("Modo Trace Desativado!\n");
        }
    } else {
        printf("Deu pau...");
    }
return machine;
}


//esse
int main() {
    setlocale(LC_ALL,"Portuguese");

    //Declaração de variáveis---------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------
    struct allInstructions programa;
    struct estadoMaquina memoriaERegs;
    struct listaLigada listaEventos;
    struct evento eventNow;
    int LECounter = 0;
    int extractorCounter = 0;
    int tamanho;
    int y = 0;
    int i = 0;
    int pushEvento = 1;
    int encerrarOperacao = 0;
    int isTrace = 0;
    int marcaVez = 0;
    char firstInstruction[50];
    char* token;
    char* token2;
    char* token3;
    const char* aux;


    memoriaERegs.enable = 1;
    //--------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------
    printf("Motor de eventos\n");
    printf("Digite o comando que desejar.\n");
    printf("Por exemplo:\n");
    printf("HE exibe lista dos comandos disponíveis;\n");
    printf("EX <nome do arquivo> carrega um programa e o executa.\n");

    while(encerrarOperacao == 0) {   //1st time ok- mas sera q roda mais de uma vez, digo, o string fica limpinho pra isso?
        printf(">>");
        scanf("%[^\n]%*c",firstInstruction);

        token = strtok(firstInstruction, " ");

        if(strcmp(token,"EX") == 0) { //inserir logica de ler nome do arquivo junto com o EX
            token = strtok(NULL, " ");
            tamanho = lerPrograma(programa.instrucaoMnemonico,token);
            programa = converterParaHexadecimal(programa,tamanho);   // supondo por enquanto que nao há uso de mnemonicos

            trueLoader(memoriaERegs.memory, programa.inicio, programa, tamanho);
            memoriaERegs.PC = programa.inicio;

            while(memoriaERegs.enable == 1) {
                if(pushEvento == 1) {
                    listaEventos.instruEvento[LECounter].instrucao[0] = memoriaERegs.memory[memoriaERegs.PC];   memoriaERegs.PC++;
                    listaEventos.instruEvento[LECounter].instrucao[1] = memoriaERegs.memory[memoriaERegs.PC];   memoriaERegs.PC++;
                    listaEventos.instruEvento[LECounter].instrucao[2] = memoriaERegs.memory[memoriaERegs.PC];   memoriaERegs.PC++;
                    listaEventos.instruEvento[LECounter].instrucao[3] = memoriaERegs.memory[memoriaERegs.PC];   memoriaERegs.PC++;
                    LECounter++;
                }

                //função de extrator: pegar um evento da lista de eventos e ter como o evento a ser executado.----------------------

                eventNow = listaEventos.instruEvento[extractorCounter];
                extractorCounter += 1;

                //Execução do evento selecionado------------------------------------------------------------------------------------
                memoriaERegs = selectReacoes(memoriaERegs,eventNow);

                //Caso modo trace ativado, dps de cada instrução, insere evento dependente MD na lista de eventos
                if(memoriaERegs.isTrace == 1 && marcaVez == 0) {
                    listaEventos.instruEvento[LECounter].instrucao[0] = 0x61;
                    listaEventos.instruEvento[LECounter].instrucao[1] = memoriaERegs.PC;
                    LECounter++;
                    pushEvento = 0;
                    marcaVez = 1;
                } else if(memoriaERegs.isTrace == 1 && marcaVez == 1) {
                    marcaVez = 0;
                    pushEvento = 1;
                }
            }

        } else if(strcmp(token,"MD") == 0) {  //evento artificial MD
            token = strtok(NULL, " \n");
            aux = token;
            listaEventos.instruEvento[LECounter].instrucao[0] = 0x61;
            listaEventos.instruEvento[LECounter].instrucao[1] = atoi(token);
            LECounter++;

            eventNow = listaEventos.instruEvento[extractorCounter];
            extractorCounter += 1;
            memoriaERegs = selectReacoes(memoriaERegs,eventNow);

            memset(firstInstruction,0,sizeof(firstInstruction));

        } else if(strcmp(token,"MM") == 0) {  //evento artificial MM          //MM <ADDR> <VALOR>
            token2 = strtok(NULL, " ");
            token3 = strtok(NULL, " ");

            listaEventos.instruEvento[LECounter].instrucao[0] = 0x62;
            listaEventos.instruEvento[LECounter].instrucao[1] = token2;
            listaEventos.instruEvento[LECounter].instrucao[2] = token3;
            LECounter++;

            eventNow = listaEventos.instruEvento[extractorCounter];
            extractorCounter++;

            memoriaERegs = selectReacoes(memoriaERegs,eventNow);

            memset(firstInstruction,0,sizeof(firstInstruction));

        } else if(strcmp(firstInstruction,"HE") == 0) { //DISPLAY DE TDS OS POSSIVEIS COMANDOS
            listaEventos.instruEvento[LECounter].instrucao[0] = 0x63;               //SENDO INSTRU, PSEUDOINSTRU OU EVENTOS ARTIFICIAIS!
            LECounter++;
            eventNow = listaEventos.instruEvento[extractorCounter];
            extractorCounter += 1;
            memoriaERegs = selectReacoes(memoriaERegs,eventNow);

            memset(firstInstruction,0,sizeof(firstInstruction));

        } else if(strcmp(firstInstruction,"END") == 0) { //finaliza td
            printf("Tchau!\n");
            break;

        } else if(strcmp(firstInstruction,"TRC") == 0) { //modo trace ativado/desativado
            listaEventos.instruEvento[LECounter].instrucao[0] = 0x64;
            LECounter++;

            eventNow = listaEventos.instruEvento[extractorCounter];
            extractorCounter++;

            memoriaERegs = selectReacoes(memoriaERegs,eventNow);

            memset(firstInstruction,0,sizeof(firstInstruction));
        } else {
            printf("Comando inválido, tente novamente!\n");
        }


        //Motor de Eventos--------------------------------------------------------------------------


    }
return 0;
}
