/* * Define o padrão POSIX para garantir que as funções 'termios.h' 
 * estejam disponíveis em sistemas Linux/macOS.
 * DEVE SER A PRIMEIRA LINHA DO ARQUIVO.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

/* --- Bloco de Detecção de Plataforma Aprimorado --- */
#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__)
    #define PLATAFORMA_WINDOWS
    #include <direct.h> // Para _mkdir
    #include <conio.h>  // Para _getch
#else
    #define PLATAFORMA_POSIX
    #include <sys/stat.h> // Para mkdir
    #include <termios.h>  // Para tcgetattr, tcsetattr
    #include <unistd.h>   // Para STDIN_FILENO
#endif
/* --- Fim do Bloco de Detecção --- */


// --- CONSTANTES GLOBAIS ---
#define NOME_ARQUIVO "tabela_usuarios.csv"
#define PASTA_EXERCICIOS "exercicios_enviados"
#define TAM_MATERIAS 8
#define TAM_TURMAS 2 // <-- MODIFICADO

// Lista de matérias fixas (Nome da constante estava OK)
const char *MATERIAS[TAM_MATERIAS] = {
    "ENGENHARIA DE SOFTWARE AGIL",
    "ALGORITIMO E ESTRUTURA DE DADOS PYTHON",
    "PROGRAMACAO ESTRUTURADA EM C",
    "ANALISE E PROJETO DE SISTEMAS",
    "PESQUISA TECNOLOGIA E INOVACAO",
    "EDUCACAO AMBIENTAL",
    "REDES DE COMPUTADORES",
    "INTELIGENCIA ARTIFICIAL"
};

// --- BLOCO MODIFICADO ---
// A coluna "TURMA" agora armazena o período.
const char *TURMAS[TAM_TURMAS] = {
    "MANHA",
    "NOITE"
};
// --- FIM DA MODIFICAÇÃO ---

// --- DECLARAÇÕES DE FUNÇÕES (PROTÓTIPOS) ---
void menuAdmin();
void menuCoordenador();
void menuProfessor();
// ATUALIZADO: Agora passa 'materia' e a nova 'turma'
void menuAluno(const char *nome, int np1, int np2, int pim, const char *materia, const char *turma);
void visualizarRelatorios();
void mostrarAlunosPorMateria(); // Renomeado (era mostrarAlunosPorTurma)
void atualizarNotas();
void gerenciarMaterias(); // Renomeado (era gerenciarTurmas)
void gerenciarUsuarios();
void excluirUsuario(const char *nivelExecutor);
void mostrarTodosUsuarios();
void cadastrarUsuarios(const char *nivelExecutor);
void cadastrarUsuariosCoordenador(); 
void cadastrarUsuarioBase(const char *nivel);
int adminExiste(); 
void loginUsuario();
void inicializarArquivoCSV();


// --- FUNÇÕES AUXILIARES DE LIMPEZA ---

void limpar_buffer_entrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void trimWhitespace(char *str) {
    if (!str) return;
    char *start = str;
    while(isspace((unsigned char)*start)) start++;
    char *end = str + strlen(str) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    if(str != start) memmove(str, start, end - start + 2);
}

void toLowerString(char *str) {
    if (!str) return;
    for(; *str; ++str) *str = tolower((unsigned char)*str);
}

void toUpperString(char *str) {
    if (!str) return;
    for(; *str; ++str) *str = toupper((unsigned char)*str);
}

// --- FUNÇÕES DE SENHA E HASH (Sem alterações) ---

void obterSenhaOculta(char *senha, int tam_max) {
    int i = 0;
    char ch;

#ifdef PLATAFORMA_WINDOWS
    while (1) {
        ch = _getch(); 
        if (ch == '\r' || ch == '\n') { 
            senha[i] = '\0';
            printf("\n");
            break;
        } else if (ch == '\b') { 
            if (i > 0) {
                i--;
                printf("\b \b"); 
            }
        } else if (i < tam_max - 1 && isprint(ch)) { 
            senha[i] = ch;
            printf("*");
            i++;
        }
    }
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); 
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON); 
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); 

    while (1) {
        ch = getchar();
        if (ch == '\n' || ch == '\r') { 
            senha[i] = '\0';
            printf("\n");
            break;
        } else if (ch == 127) { 
            if (i > 0) {
                i--;
                printf("\b \b"); 
            }
        } else if (i < tam_max - 1 && isprint(ch)) { 
            senha[i] = ch;
            printf("*");
            i++;
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); 
#endif
}
void hashSenha(const char *senha_plain, char *senha_hash_out, int tam_hash) {
    unsigned long hash = 5381; 
    int c;
    while ((c = *senha_plain++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    snprintf(senha_hash_out, tam_hash, "%lx", hash);
}
int obterMaiorID() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) return 0;
    char linha[300];
    int maiorID = 0, id;
    fgets(linha, sizeof(linha), arquivo); 
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (sscanf(linha, "%d,", &id) == 1)
            if (id > maiorID) maiorID = id;
    }
    fclose(arquivo);
    return maiorID;
}

// --- FUNÇÕES PRINCIPAIS DO SISTEMA ---

// (adminExiste não precisou de alteração, pois só lê até a coluna 6)
int adminExiste() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) return 0; 

    char linha[400];
    char nivel[20];
    int adminEncontrado = 0;

    fgets(linha, sizeof(linha), arquivo); // Pula cabeçalho

    while (fgets(linha, sizeof(linha), arquivo)) {
        char linha_copia[400];
        strncpy(linha_copia, linha, sizeof(linha_copia)-1);
        linha_copia[sizeof(linha_copia)-1] = '\0';
        linha_copia[strcspn(linha_copia, "\r\n")] = 0;
        
        char *saveptr;
        char *tok;

        #ifdef PLATAFORMA_WINDOWS
            #define TOK_R(str, delim, save) strtok_s(str, delim, save)
        #else
            #define TOK_R(str, delim, save) strtok_r(str, delim, save)
        #endif

        tok = TOK_R(linha_copia, ",", &saveptr); if (!tok) continue; // ID
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // NOME
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // EMAIL
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // SENHA
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // IDADE
        
        tok = TOK_R(NULL, ",", &saveptr); 
        if (!tok) continue;
        strncpy(nivel, tok, sizeof(nivel)-1); nivel[sizeof(nivel)-1] = '\0';

        trimWhitespace(nivel);
        toLowerString(nivel);

        if (strcmp(nivel, "admin") == 0) {
            adminEncontrado = 1;
            break; 
        }
    }

    fclose(arquivo);
    return adminEncontrado;
}

// --- CADASTRO DE USUÁRIOS (REFACHINADO E COM HASH) ---

/*
 * @brief (ATUALIZADO) Função base que REALIZA o cadastro.
 * Agora salva MATERIA e a nova coluna TURMA.
 */
/*
 * @brief (ATUALIZADO) Função base que REALIZA o cadastro.
 * Agora salva MATERIA e a nova coluna TURMA (selecionada de uma lista).
 */
/*
 * @brief (ATUALIZADO) Função base que REALIZA o cadastro.
 * Atribui 'Curso' automaticamente e permite selecionar Matéria e Turma.
 */
void cadastrarUsuarioBase(const char *nivel) {
    FILE *arquivo;
    char nome[50], email[50], senha_plain[20], senha_hash[20], curso[50], materia[100], turma[100];
    int idade, np1 = 0, np2 = 0, pim = 0;

    int proximoID = obterMaiorID() + 1;
    arquivo = fopen(NOME_ARQUIVO, "a");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo para escrita.\n");
        return;
    }

    printf("\n=== Cadastro de %s ===\n", nivel);
    printf("Nome: "); fgets(nome, sizeof(nome), stdin); nome[strcspn(nome, "\n")] = '\0';
    printf("Email: "); fgets(email, sizeof(email), stdin); email[strcspn(email, "\n")] = '\0';
    
    printf("Senha: "); 
    obterSenhaOculta(senha_plain, sizeof(senha_plain)); 
    
    printf("Idade: ");
    if (scanf("%d", &idade) != 1) {
        printf("Idade inválida.\n");
        limpar_buffer_entrada();
        fclose(arquivo);
        return;
    }
    limpar_buffer_entrada(); // <-- Limpa o buffer após o scanf da idade

    // Limpa os dados (Nome e Email)
    trimWhitespace(nome);
    trimWhitespace(email);

    // Lógica para Aluno (Matéria e Turma)
    if (strcmp(nivel, "aluno") == 0) {
        
        // --- INÍCIO DA MODIFICAÇÃO ---
        // 1. Atribuir o Curso (Fixo)
        strcpy(curso, "ANALISE E DESENVOLVIMENTO DE SISTEMAS");
        printf("\nCurso atribuído: %s\n", curso);
        // --- FIM DA MODIFICAÇÃO ---

        // 2. Selecionar a Matéria
        printf("\nEscolha a MATÉRIA:\n");
        for (int i = 0; i < TAM_MATERIAS; i++) {
            printf("%d - %s\n", i + 1, MATERIAS[i]);
        }
        int opc_materia;
        printf("Opção: ");
        if (scanf("%d", &opc_materia) != 1) {
            limpar_buffer_entrada();
            strcpy(materia, "SEM MATERIA");
        } else {
            limpar_buffer_entrada();
            if (opc_materia >= 1 && opc_materia <= TAM_MATERIAS) {
                strcpy(materia, MATERIAS[opc_materia - 1]);
            } else {
                strcpy(materia, "SEM MATERIA");
            }
        }
        
        // 3. Selecionar a Turma (Período)
        printf("\nEscolha a TURMA (Período):\n");
        for (int i = 0; i < TAM_TURMAS; i++) {
            printf("%d - %s\n", i + 1, TURMAS[i]);
        }
        
        int opc_turma;
        printf("Opção: ");
        if (scanf("%d", &opc_turma) != 1) {
            limpar_buffer_entrada();
            strcpy(turma, "SEM TURMA");
        } else {
            limpar_buffer_entrada();
            if (opc_turma >= 1 && opc_turma <= TAM_TURMAS) {
                strcpy(turma, TURMAS[opc_turma - 1]);
            } else {
                printf("Opção inválida. 'SEM TURMA' será atribuído.\n");
                strcpy(turma, "SEM TURMA");
            }
        }

    } else {
        // Professores, Admins, etc.
        // --- INÍCIO DA MODIFICAÇÃO ---
        strcpy(curso, "-"); // Atribui "-" para não-alunos
        // --- FIM DA MODIFICAÇÃO ---
        strcpy(materia, "-"); 
        strcpy(turma, "-");   
    }

    hashSenha(senha_plain, senha_hash, sizeof(senha_hash));

    // Salva 12 colunas
    fprintf(arquivo, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d,%s,%s\n",
            proximoID, nome, email, senha_hash, idade, nivel, curso, 
            np1, np2, pim, materia, turma);
    fclose(arquivo);
    printf("\n✅ Usuário cadastrado com sucesso!\n");
}

// (Função sem alteração de lógica, apenas de permissão)
void cadastrarUsuarios(const char *nivelExecutor) {
    int opcaoNivel;
    char nivel[20];

    printf("\nSelecione o nível de acesso:\n");
    printf("1 - Admin\n2 - Coordenador\n3 - Professor\n4 - Aluno\n");
    printf("Opção: ");

    if (scanf("%d", &opcaoNivel) != 1) {
        printf("Entrada inválida.\n");
        limpar_buffer_entrada();
        return;
    }
    limpar_buffer_entrada();

    switch (opcaoNivel) {
        case 1:
            if (strcmp(nivelExecutor, "admin") != 0) {
                if (adminExiste()) {
                    printf("\n❌ Erro: Já existe um usuário 'admin' no sistema.\n");
                    printf("Apenas um administrador logado pode cadastrar outro admin.\n");
                    return; 
                }
            }
            strcpy(nivel, "admin");
            break;
            
        case 2: 
            strcpy(nivel, "coordenador"); 
            break;
        case 3: 
            strcpy(nivel, "professor"); 
            break;
        case 4: 
            strcpy(nivel, "aluno"); 
            break;
        default: 
            printf("Opção inválida.\n"); 
            return;
    }

    cadastrarUsuarioBase(nivel);
}

// (Função sem alteração)
void cadastrarUsuariosCoordenador() {
    int opcaoNivel;
    char nivel[20];
    
    printf("\n=== CADASTRAR (COORDENADOR) ===\n");
    printf("Selecione o nível de acesso para o novo usuário:\n");
    printf("1 - Professor\n");
    printf("2 - Aluno\n");
    printf("0 - Cancelar\n");
    printf("Opção: ");
    
    if (scanf("%d", &opcaoNivel) != 1) {
        printf("Entrada inválida.\n");
        limpar_buffer_entrada();
        return;
    }
    limpar_buffer_entrada();

    switch (opcaoNivel) {
        case 1:
            strcpy(nivel, "professor");
            break;
        case 2:
            strcpy(nivel, "aluno");
            break;
        case 0:
            printf("Cadastro cancelado.\n");
            return; 
        default:
            printf("Opção inválida.\n");
            return; 
    }
    
    cadastrarUsuarioBase(nivel);
}

// (excluirUsuario não precisou de alteração, pois só lê até a coluna 6)
void excluirUsuario(const char *nivelExecutor) {
    FILE *arquivo, *temp;
    char linha[400];
    int idBusca, id;
    char nome[50], nivel_alvo[20];
    int encontrado = 0;
    int excluido = 0;

    printf("\n=== EXCLUIR USUÁRIO ===\n");
    printf("Digite o ID do usuário a ser excluído: ");
    if (scanf("%d", &idBusca) != 1) {
        printf("ID inválido.\n");
        limpar_buffer_entrada();
        return;
    }
    limpar_buffer_entrada();

    arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Erro: Arquivo %s não encontrado.\n", NOME_ARQUIVO);
        return;
    }
    temp = fopen("temp.csv", "w");
    if (!temp) {
        printf("Erro: Não foi possível criar arquivo temporário.\n");
        fclose(arquivo);
        return;
    }

    if (fgets(linha, sizeof(linha), arquivo)) {
        fprintf(temp, "%s", linha);
    }

    #ifdef PLATAFORMA_WINDOWS
        #define TOK_R(str, delim, save) strtok_s(str, delim, save)
    #else
        #define TOK_R(str, delim, save) strtok_r(str, delim, save)
    #endif

    while (fgets(linha, sizeof(linha), arquivo)) {
        char linha_copia[400];
        strncpy(linha_copia, linha, sizeof(linha_copia)-1);
        linha_copia[sizeof(linha_copia)-1] = '\0';
        linha_copia[strcspn(linha_copia, "\r\n")] = 0;
        char *saveptr;
        char *tok;

        tok = TOK_R(linha_copia, ",", &saveptr);
        if (!tok) { fprintf(temp, "%s", linha); continue; }
        id = atoi(tok);

        tok = TOK_R(NULL, ",", &saveptr);
        if (!tok) { fprintf(temp, "%s", linha); continue; }
        strncpy(nome, tok, sizeof(nome)-1); nome[sizeof(nome)-1] = '\0';
        
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; } // EMAIL
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; } // SENHA
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; } // IDADE

        tok = TOK_R(NULL, ",", &saveptr);
        if (!tok) { fprintf(temp, "%s", linha); continue; }
        strncpy(nivel_alvo, tok, sizeof(nivel_alvo)-1); nivel_alvo[sizeof(nivel_alvo)-1] = '\0';

        if (id == idBusca) {
            encontrado = 1;
            trimWhitespace(nivel_alvo);
            toLowerString(nivel_alvo);
            int podeExcluir = 0; 

            if (strcmp(nivelExecutor, "admin") == 0) {
                if (strcmp(nivel_alvo, "admin") == 0) {
                    printf("❌ ERRO: Um Admin não pode excluir outro usuário 'admin'.\n");
                } else {
                    podeExcluir = 1; 
                }
            } 
            else if (strcmp(nivelExecutor, "coordenador") == 0) {
                if (strcmp(nivel_alvo, "professor") == 0 || strcmp(nivel_alvo, "aluno") == 0) {
                    podeExcluir = 1; 
                } else {
                    printf("❌ PERMISSÃO NEGADA: Coordenador não pode excluir usuários de nível '%s'.\n", nivel_alvo);
                }
            }
            if (podeExcluir) {
                printf("✅ Usuário '%s' (Nível: %s) foi excluído com sucesso.\n", nome, nivel_alvo);
                excluido = 1;
            } else {
                fprintf(temp, "%s", linha);
            }
        } else {
            fprintf(temp, "%s", linha);
        }
    }
    fclose(arquivo);
    fclose(temp);
    remove(NOME_ARQUIVO);
    rename("temp.csv", NOME_ARQUIVO);
    if (!encontrado) {
        printf("⚠️ Usuário com ID %d não foi encontrado.\n", idBusca);
    }
}

/**
 * @brief (ATUALIZADO) Atualiza notas.
 * Agora lê/escreve 12 colunas.
 */
void atualizarNotas() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Arquivo não encontrado.\n");
        return;
    }
    FILE *temp = fopen("temp.csv", "w");
    if (!temp) {
        fclose(arquivo);
        printf("Erro ao criar arquivo temporário.\n");
        return;
    }

    char linha[400];
    int idBusca, id, idade, np1, np2, pim;
    // ATUALIZADO: 'turma' virou 'materia' e 'turma' foi adicionada
    char nome[50], email[50], senha[20], nivel[20], curso[50], materia[100], turma[100];
    int encontrado = 0;

    if(fgets(linha, sizeof(linha), arquivo) == NULL) {
         printf("Arquivo vazio.\n");
         fclose(arquivo); fclose(temp); remove("temp.csv");
         return;
    }
    fprintf(temp, "%s", linha);

    printf("Digite o ID do aluno: ");
    if(scanf("%d", &idBusca) != 1) {
        printf("ID inválido.\n");
        limpar_buffer_entrada();
        fclose(arquivo); fclose(temp); remove("temp.csv");
        return;
    }
    limpar_buffer_entrada();

    #ifdef PLATAFORMA_WINDOWS
        #define TOK_R(str, delim, save) strtok_s(str, delim, save)
    #else
        #define TOK_R(str, delim, save) strtok_r(str, delim, save)
    #endif

    while (fgets(linha, sizeof(linha), arquivo)) {
        char linha_copia[400];
        strncpy(linha_copia, linha, sizeof(linha_copia)-1);
        linha_copia[sizeof(linha_copia)-1] = '\0';
        linha_copia[strcspn(linha_copia, "\r\n")] = 0;
        char *saveptr;
        char *tok;

        // --- Parse de 12 colunas ---
        tok = TOK_R(linha_copia, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; }
        id = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; }
        strncpy(nome, tok, sizeof(nome)-1); nome[sizeof(nome)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; }
        strncpy(email, tok, sizeof(email)-1); email[sizeof(email)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; }
        strncpy(senha, tok, sizeof(senha)-1); senha[sizeof(senha)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; }
        idade = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; }
        strncpy(nivel, tok, sizeof(nivel)-1); nivel[sizeof(nivel)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(curso, tok, sizeof(curso)-1); curso[sizeof(curso)-1] = '\0'; } 
        else { curso[0] = '\0'; }
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; }
        np1 = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; }
        np2 = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) { fprintf(temp, "%s", linha); continue; }
        pim = atoi(tok);
        // ATUALIZADO: Coluna 11 (Materia)
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(materia, tok, sizeof(materia)-1); materia[sizeof(materia)-1] = '\0'; } 
        else { materia[0] = '\0'; }
        // ATUALIZADO: Coluna 12 (Turma)
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(turma, tok, sizeof(turma)-1); turma[sizeof(turma)-1] = '\0'; } 
        else { turma[0] = '\0'; }
        // --- Fim do Parse ---
        
        char nivel_temp[20];
        strcpy(nivel_temp, nivel);
        trimWhitespace(nivel_temp);
        toLowerString(nivel_temp);
        
        if (id == idBusca && strcmp(nivel_temp, "aluno") == 0) {
            printf("Aluno encontrado: %s\n", nome);
            printf("Nova NP1 (Atual: %d): ", np1); scanf("%d", &np1);
            printf("Nova NP2 (Atual: %d): ", np2); scanf("%d", &np2);
            printf("Novo PIM (Atual: %d): ", pim); scanf("%d", &pim);
            limpar_buffer_entrada();
            encontrado = 1;
        }
        
        // ATUALIZADO: Reescreve 12 colunas
        fprintf(temp, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d,%s,%s\n",
                 id, nome, email, senha, idade, nivel, curso, np1, np2, pim, materia, turma);
    }

    fclose(arquivo);
    fclose(temp);
    remove(NOME_ARQUIVO);
    rename("temp.csv", NOME_ARQUIVO);

    if (encontrado)
        printf("✅ Notas atualizadas com sucesso!\n");
    else
        printf("⚠️ Aluno não encontrado ou ID pertence a um usuário que não é aluno.\n");
}

// (Função sem alteração)
void enviarExercicioPDF() {
    char caminhoPDF[260], destino[300], nomeArquivo[100];
    FILE *origem, *destinoF;

#ifdef PLATAFORMA_WINDOWS
    _mkdir(PASTA_EXERCICIOS);
#else
    mkdir(PASTA_EXERCICIOS, 0777);
#endif

    printf("\nDigite o caminho do arquivo PDF a enviar: ");
    fgets(caminhoPDF, sizeof(caminhoPDF), stdin);
    caminhoPDF[strcspn(caminhoPDF, "\n")] = '\0';

    const char *nome = strrchr(caminhoPDF, '\\');
    if (!nome) nome = strrchr(caminhoPDF, '/');
    nome = nome ? nome + 1 : caminhoPDF;
    
    strncpy(nomeArquivo, nome, sizeof(nomeArquivo) - 1);
    nomeArquivo[sizeof(nomeArquivo) - 1] = '\0';
    snprintf(destino, sizeof(destino), "%s/%s", PASTA_EXERCICIOS, nomeArquivo);

    origem = fopen(caminhoPDF, "rb");
    if (!origem) {
        printf("❌ Erro: arquivo não encontrado.\n");
        return;
    }
    destinoF = fopen(destino, "wb");
    if (!destinoF) {
        fclose(origem);
        printf("❌ Erro ao criar o arquivo destino.\n");
        return;
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), origem)) > 0)
        fwrite(buffer, 1, bytes, destinoF);

    fclose(origem);
    fclose(destinoF);
    printf("✅ Exercício enviado para %s\n", destino);
}

/**
 * @brief (ATUALIZADO) Mostra todos os usuários.
 * Agora lê 12 colunas e exibe MATERIA e TURMA.
 */
void mostrarTodosUsuarios() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Nenhum usuário cadastrado.\n");
        return;
    }

    char linha[400];
    int id, idade, np1, np2, pim;
    // ATUALIZADO: 'turma' virou 'materia' e 'turma' foi adicionada
    char nome[50], email[50], senha[20], nivel[20], curso[50], materia[100], turma[100];

    printf("\n=== USUÁRIOS CADASTRADOS ===\n");
    
    if (fgets(linha, sizeof(linha), arquivo)) {
        // --- CABEÇALHO ATUALIZADO (12 colunas) ---
        printf("----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        printf("| ID | NOME                   | EMAIL                | NIVEL       | IDADE | CURSO                | NP1 | NP2 | PIM | MÉDIA | STATUS    | MATÉRIA                              | TURMA                |\n");
        printf("----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    }
    
    #ifdef PLATAFORMA_WINDOWS
        #define TOK_R(str, delim, save) strtok_s(str, delim, save)
    #else
        #define TOK_R(str, delim, save) strtok_r(str, delim, save)
    #endif

    while (fgets(linha, sizeof(linha), arquivo)) {
        char linha_copia[400];
        strncpy(linha_copia, linha, sizeof(linha_copia)-1);
        linha_copia[sizeof(linha_copia)-1] = '\0';
        linha_copia[strcspn(linha_copia, "\r\n")] = 0;
        char *saveptr;
        char *tok;

        // --- Parse de 12 colunas ---
        tok = TOK_R(linha_copia, ",", &saveptr); if (!tok) continue;
        id = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(nome, tok, sizeof(nome)-1); nome[sizeof(nome)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(email, tok, sizeof(email)-1); email[sizeof(email)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(senha, tok, sizeof(senha)-1); senha[sizeof(senha)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        idade = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(nivel, tok, sizeof(nivel)-1); nivel[sizeof(nivel)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(curso, tok, sizeof(curso)-1); curso[sizeof(curso)-1] = '\0'; } 
        else { curso[0] = '\0'; }
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        np1 = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        np2 = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        pim = atoi(tok);
        // ATUALIZADO: Coluna 11 (Materia)
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(materia, tok, sizeof(materia)-1); materia[sizeof(materia)-1] = '\0'; } 
        else { materia[0] = '\0'; }
        // ATUALIZADO: Coluna 12 (Turma)
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(turma, tok, sizeof(turma)-1); turma[sizeof(turma)-1] = '\0'; } 
        else { turma[0] = '\0'; }
        // --- Fim do Parse ---
        
        char media_str[10] = "N/A";
        char status_str[12] = "N/A";
        char nivel_temp[20];
        
        strcpy(nivel_temp, nivel);
        trimWhitespace(nivel_temp);
        toLowerString(nivel_temp);

        if (strcmp(nivel_temp, "aluno") == 0) {
            float media = (np1 * 4 + np2 * 4 + pim * 2) / 10.0;
            sprintf(media_str, "%.2f", media); 
            if (media >= 7.0) {
                strcpy(status_str, "APROVADO");
            } else {
                strcpy(status_str, "REPROVADO");
            }
        }
        
        // --- PRINTF ATUALIZADO (12 colunas) ---
        printf("| %-2d | %-22s | %-20s | %-11s | %-5d | %-20s | %-3d | %-3d | %-3d | %-5s | %-9s | %-36s | %-20s |\n",
                 id, nome, email, nivel, idade, curso, np1, np2, pim, 
                 media_str, status_str, materia, turma); 
    }
    
    printf("----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    fclose(arquivo);
}


/**
 * @brief (ATUALIZADO) Gera relatório de notas por MATÉRIA.
 */
void visualizarRelatorios() {
    // ATUALIZADO: Variável renomeada
    char materia_escolhida[100];
    int opc;

    printf("\n=== RELATÓRIO DE NOTAS POR MATÉRIA ===\n"); // <-- Texto corrigido
    printf("Escolha a MATÉRIA para gerar o relatório:\n"); // <-- Texto corrigido
    
    for (int i = 0; i < TAM_MATERIAS; i++) {
        printf("%d - %s\n", i + 1, MATERIAS[i]);
    }
    printf("Opção: ");

    if (scanf("%d", &opc) != 1) {
        limpar_buffer_entrada(); 
        printf("Erro: Entrada inválida. Por favor, digite um número.\n");
        return;
    }
    limpar_buffer_entrada(); 

    if (opc < 1 || opc > TAM_MATERIAS) {
        printf("Opção inválida.\n");
        return;
    }
    
    strcpy(materia_escolhida, MATERIAS[opc - 1]); // <-- Variável corrigida

    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo de usuários.\n");
        return;
    }

    char linha[400];
    // ATUALIZADO: 'turma_lida' virou 'materia_lida'
    char nome[50], nivel[20], materia_lida[100];
    int id, np1, np2, pim;
    int alunos_encontrados = 0;
    float soma_medias_turma = 0.0;

    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        printf("Arquivo de usuários está vazio ou ilegível.\n");
        fclose(arquivo);
        return;
    }

    printf("\n--- Relatório de Notas: %s ---\n", materia_escolhida);
    printf("------------------------------------------------------------------------------\n");
    printf("| ID | NOME                   | NP1 | NP2 | PIM | MÉDIA | STATUS    |\n");
    printf("------------------------------------------------------------------------------\n");

    #ifdef PLATAFORMA_WINDOWS
        #define TOK_R(str, delim, save) strtok_s(str, delim, save)
    #else
        #define TOK_R(str, delim, save) strtok_r(str, delim, save)
    #endif

    while (fgets(linha, sizeof(linha), arquivo)) {
        char linha_copia[400];
        strncpy(linha_copia, linha, sizeof(linha_copia)-1);
        linha_copia[sizeof(linha_copia)-1] = '\0';
        linha_copia[strcspn(linha_copia, "\r\n")] = 0;
        char *saveptr;
        char *tok;

        // --- Parse (só precisamos da col 11) ---
        tok = TOK_R(linha_copia, ",", &saveptr); if (!tok) continue;
        id = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(nome, tok, sizeof(nome)-1); nome[sizeof(nome)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // EMAIL
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // SENHA
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // IDADE
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(nivel, tok, sizeof(nivel)-1); nivel[sizeof(nivel)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // CURSO
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        np1 = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        np2 = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        pim = atoi(tok);
        // ATUALIZADO: Coluna 11 (Materia)
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(materia_lida, tok, sizeof(materia_lida)-1); materia_lida[sizeof(materia_lida)-1] = '\0'; } 
        else { materia_lida[0] = '\0'; }
        // (Coluna 12 não é necessária aqui)
        // --- Fim do Parse ---
        
        materia_lida[strcspn(materia_lida, "\n")] = '\0';
        trimWhitespace(nivel);
        toLowerString(nivel);
        trimWhitespace(materia_lida);
        toUpperString(materia_lida); 
        
        // ATUALIZADO: Lógica compara 'materia_lida' com 'materia_escolhida'
        if (strcmp(nivel, "aluno") == 0 && strcmp(materia_lida, materia_escolhida) == 0) {
            alunos_encontrados++;
            float media_individual = (np1 * 4 + np2 * 4 + pim * 2) / 10.0;
            soma_medias_turma += media_individual;
            
            const char *status;
            if (media_individual >= 7.0) {
                status = "APROVADO";
            } else {
                status = "REPROVADO";
            }
            
            printf("| %-2d | %-22s | %-3d | %-3d | %-3d | %-5.2f | %-9s |\n",
                   id, nome, np1, np2, pim, media_individual, status);
        }
    }

    printf("------------------------------------------------------------------------------\n");

    if (alunos_encontrados == 0) {
        printf("Nenhum aluno encontrado para a matéria '%s'.\n", materia_escolhida);
    } else {
        float media_geral_turma = soma_medias_turma / alunos_encontrados;
        char texto_media[100];
        sprintf(texto_media, "Média Geral da Matéria: %.2f", media_geral_turma); // <-- Texto corrigido

        printf("| %-76s |\n", texto_media);
        printf("------------------------------------------------------------------------------\n");
        printf("Total: %d aluno(s) encontrado(s).\n", alunos_encontrados);
    }
    
    fclose(arquivo);
    printf("\nPressione ENTER para voltar ao menu...");
    getchar(); 
}

/**
 * @brief (ATUALIZADO) Lista alunos por MATÉRIA e agora mostra a TURMA.
 */
void mostrarAlunosPorMateria() { // <-- Renomeado
    char materia_escolhida[100]; // <-- Renomeado
    int opc;

    printf("\n=== MOSTRAR ALUNOS POR MATÉRIA ===\n"); // <-- Texto corrigido
    printf("Escolha a MATÉRIA para listar os alunos:\n"); // <-- Texto corrigido
    
    for (int i = 0; i < TAM_MATERIAS; i++) {
        printf("%d - %s\n", i + 1, MATERIAS[i]);
    }
    printf("Opção: ");

    if (scanf("%d", &opc) != 1) {
        limpar_buffer_entrada(); 
        printf("Erro: Entrada inválida. Por favor, digite um número.\n");
        return;
    }
    limpar_buffer_entrada(); 

    if (opc < 1 || opc > TAM_MATERIAS) {
        printf("Opção inválida.\n");
        return;
    }
    
    strcpy(materia_escolhida, MATERIAS[opc - 1]); // <-- Renomeado

    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo de usuários.\n");
        return;
    }

    char linha[400];
    // ATUALIZADO: 'turma_lida' virou 'materia_lida', 'turma_lida' é a nova coluna
    char nome[50], email[50], nivel[20], curso[50], materia_lida[100], turma_lida[100];
    int id;
    int alunos_encontrados = 0;

    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        printf("Arquivo de usuários está vazio ou ilegível.\n");
        fclose(arquivo);
        return;
    }

    printf("\n--- Alunos da Matéria: %s ---\n", materia_escolhida); // <-- Texto corrigido
    // --- CABEÇALHO ATUALIZADO (com TURMA) ---
    printf("--------------------------------------------------------------------------------------------------------\n");
    printf("| ID | NOME                   | EMAIL                                | CURSO                | TURMA                |\n");
    printf("--------------------------------------------------------------------------------------------------------\n");

    #ifdef PLATAFORMA_WINDOWS
        #define TOK_R(str, delim, save) strtok_s(str, delim, save)
    #else
        #define TOK_R(str, delim, save) strtok_r(str, delim, save)
    #endif

    while (fgets(linha, sizeof(linha), arquivo)) {
        char linha_copia[400];
        strncpy(linha_copia, linha, sizeof(linha_copia)-1);
        linha_copia[sizeof(linha_copia)-1] = '\0';
        linha_copia[strcspn(linha_copia, "\r\n")] = 0;
        char *saveptr;
        char *tok;

        // --- Parse de 12 colunas ---
        tok = TOK_R(linha_copia, ",", &saveptr); if (!tok) continue;
        id = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(nome, tok, sizeof(nome)-1); nome[sizeof(nome)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(email, tok, sizeof(email)-1); email[sizeof(email)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // SENHA
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // IDADE
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(nivel, tok, sizeof(nivel)-1); nivel[sizeof(nivel)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(curso, tok, sizeof(curso)-1); curso[sizeof(curso)-1] = '\0'; } 
        else { curso[0] = '\0'; }
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // NP1
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // NP2
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue; // PIM
        // ATUALIZADO: Coluna 11 (Materia)
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(materia_lida, tok, sizeof(materia_lida)-1); materia_lida[sizeof(materia_lida)-1] = '\0'; } 
        else { materia_lida[0] = '\0'; }
        // ATUALIZADO: Coluna 12 (Turma)
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(turma_lida, tok, sizeof(turma_lida)-1); turma_lida[sizeof(turma_lida)-1] = '\0'; } 
        else { turma_lida[0] = '\0'; }
        // --- Fim do Parse ---
        
        materia_lida[strcspn(materia_lida, "\n")] = '\0';
        trimWhitespace(nivel);
        toLowerString(nivel);
        trimWhitespace(materia_lida);
        toUpperString(materia_lida); 
        
        // ATUALIZADO: Lógica compara 'materia_lida'
        if (strcmp(nivel, "aluno") == 0 && strcmp(materia_lida, materia_escolhida) == 0) {
            alunos_encontrados++;
            
            // ATUALIZADO: printf agora inclui a 'turma_lida'
            printf("| %-2d | %-22s | %-36s | %-20s | %-20s |\n", 
                   id, nome, email, curso, turma_lida);
        }
    }

    printf("--------------------------------------------------------------------------------------------------------\n");
    if (alunos_encontrados == 0) {
        printf("Nenhum aluno encontrado para a matéria '%s'.\n", materia_escolhida);
    } else {
        printf("Total: %d aluno(s) encontrado(s).\n", alunos_encontrados);
    }
    
    fclose(arquivo);
    printf("\nPressione ENTER para voltar ao menu...");
    getchar(); 
}


// --- MENUS E GERENCIAMENTO ---

// (Função sem alteração)
void gerenciarUsuarios() {
    int op;
    do {
        printf("\n=== GERENCIAR USUÁRIOS ===\n");
        printf("1 - Mostrar todos os usuários\n");
        printf("2 - Cadastrar novo usuário\n");
        printf("3 - Excluir usuário\n");
        printf("0 - Voltar ao Menu Admin\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1;
        limpar_buffer_entrada();

        switch (op) {
            case 1: mostrarTodosUsuarios(); break;
            case 2: cadastrarUsuarios("admin"); break; 
            case 3: excluirUsuario("admin"); break; 
            case 0: break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

// (ATUALIZADO) Renomeado para gerenciarMaterias
void gerenciarMaterias() {
    printf("\n=== MATÉRIAS DO SISTEMA ===\n"); // <-- Texto corrigido
    for (int i = 0; i < TAM_MATERIAS; i++) {
        printf("%d - %s\n", i + 1, MATERIAS[i]);
    }
    printf("===========================\n");
    
    printf("Pressione ENTER para voltar...");
    getchar(); 
}

// (ATUALIZADO) Menu Admin chama a função renomeada
void menuAdmin() {
    int op;
    do {
        printf("\n=== MENU ADMIN ===\n");
        printf("1 - Gerenciar todos usuários (Coordenador, Professor, Aluno)\n");
        printf("2 - Gerenciar matérias\n"); // <-- Texto corrigido
        printf("3 - Atualizar notas de aluno\n");
        printf("0 - Sair\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1;
        limpar_buffer_entrada();

        switch (op) {
            case 1: gerenciarUsuarios(); break;
            case 2: gerenciarMaterias(); break; // <-- Chamada corrigida
            case 3: atualizarNotas(); break;
            case 0: printf("Saindo do menu admin...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

// (Função sem alteração)
void menuProfessor() {
    int op;
    do {
        printf("\n=== MENU PROFESSOR ===\n");
        printf("1 - Atualizar notas\n");
        printf("2 - Enviar exercício (PDF)\n");
        printf("0 - Sair\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1;
        limpar_buffer_entrada();

        switch (op) {
            case 1: atualizarNotas(); break;
            case 2: enviarExercicioPDF(); break;
            case 0: printf("Saindo do menu professor...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

// (ATUALIZADO) Menu Coordenador chama funções renomeadas
void menuCoordenador() {
    int op;
    do {
        printf("\n=== MENU COORDENADOR ===\n");
        printf("1 - Cadastrar (Professor ou Aluno)\n");
        printf("2 - Atualizar Notas de Aluno\n");
        printf("3 - Visualizar Relatórios (Notas por Matéria)\n"); // <-- Texto corrigido
        printf("4 - Mostrar Alunos por Matéria\n"); // <-- Texto corrigido
        printf("5 - Excluir Usuário (Professor ou Aluno)\n"); 
        printf("0 - Sair\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1;
        limpar_buffer_entrada(); 

        switch (op) {
            case 1:
                cadastrarUsuariosCoordenador();
                break;
            case 2:
                atualizarNotas();
                break;
            case 3:
                visualizarRelatorios();
                break;
            case 4:
                mostrarAlunosPorMateria(); // <-- Chamada corrigida
                break;
            case 5:
                excluirUsuario("coordenador"); 
                break;
            case 0:
                printf("Saindo do menu coordenador...\n");
                break;
            default:
                printf("Opção inválida.\n");
        }
    } while (op != 0);
}

/**
 * @brief (ATUALIZADO) Menu do Aluno.
 * Agora recebe 'materia' e 'turma' separadamente.
 */
void menuAluno(const char *nome, int np1, int np2, int pim, const char *materia, const char *turma) {
    int op;
    do {
        printf("\n=== MENU ALUNO (%s) ===\n", nome);
        printf("1 - Ver notas e média\n");
        printf("2 - Acessar vídeo-aula da matéria\n");
        printf("0 - Sair\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1;
        limpar_buffer_entrada();

        switch (op) {
            case 1: {
                float media = (np1 * 4 + np2 * 4 + pim * 2) / 10.0;
                const char *status;
                if (media >= 7.0) {
                    status = "APROVADO";
                } else {
                    status = "REPROVADO";
                }
                
                printf("\n--- Minhas Notas ---\n");
                printf("Matéria: %s\n", materia); // <-- Mostra a matéria
                printf("Turma: %s\n", turma);   // <-- Mostra a nova turma
                printf("NP1: %d\n", np1);
                printf("NP2: %d\n", np2);
                printf("PIM: %d\n", pim);
                printf("--------------------\n");
                printf("Média Final: %.2f\n", media);
                printf("Status: %s\n", status); 
                printf("--------------------\n");
                break;
            }
            case 2:
                // ATUALIZADO: Acessa a matéria, não a turma
                printf("\n🎥 Acessando vídeo-aula da MATÉRIA: %s\n", materia);
                printf("URL: https://www.youtube.com/results?search_query=%s\n", materia);
                break;
            case 0: printf("Saindo do menu aluno...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

/**
 * @brief (ATUALIZADO) Função de Login.
 * Agora lê 12 colunas e passa 'materia' e 'turma' para o menuAluno.
 */
void loginUsuario() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Nenhum usuário cadastrado.\n");
        return;
    }

    char email_login[50], senha_login_plain[20], senha_login_hash[20];
    char linha[400];
    // ATUALIZADO: 'turma' virou 'materia', 'turma' é nova
    char nome[50], email[50], senha_do_arquivo[20], nivel[20], curso[50], materia[100], turma[100];
    int id, idade, np1, np2, pim;
    int encontrado = 0;

    printf("\nEmail: "); fgets(email_login, sizeof(email_login), stdin);
    email_login[strcspn(email_login, "\n")] = '\0';
    
    printf("Senha: "); 
    obterSenhaOculta(senha_login_plain, sizeof(senha_login_plain));

    trimWhitespace(email_login);
    hashSenha(senha_login_plain, senha_login_hash, sizeof(senha_login_hash));

    fgets(linha, sizeof(linha), arquivo); // pula cabeçalho

    #ifdef PLATAFORMA_WINDOWS
        #define TOK_R(str, delim, save) strtok_s(str, delim, save)
    #else
        #define TOK_R(str, delim, save) strtok_r(str, delim, save)
    #endif

    while (fgets(linha, sizeof(linha), arquivo)) {
        char linha_copia[400];
        strncpy(linha_copia, linha, sizeof(linha_copia)-1);
        linha_copia[sizeof(linha_copia)-1] = '\0';
        linha_copia[strcspn(linha_copia, "\r\n")] = 0;
        char *saveptr;
        char *tok;

        // --- Parse de 12 colunas ---
        tok = TOK_R(linha_copia, ",", &saveptr); if (!tok) continue;
        id = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(nome, tok, sizeof(nome)-1); nome[sizeof(nome)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(email, tok, sizeof(email)-1); email[sizeof(email)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(senha_do_arquivo, tok, sizeof(senha_do_arquivo)-1); senha_do_arquivo[sizeof(senha_do_arquivo)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        idade = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        strncpy(nivel, tok, sizeof(nivel)-1); nivel[sizeof(nivel)-1] = '\0';
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(curso, tok, sizeof(curso)-1); curso[sizeof(curso)-1] = '\0'; } 
        else { curso[0] = '\0'; }
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        np1 = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        np2 = atoi(tok);
        tok = TOK_R(NULL, ",", &saveptr); if (!tok) continue;
        pim = atoi(tok);
        // ATUALIZADO: Coluna 11 (Materia)
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(materia, tok, sizeof(materia)-1); materia[sizeof(materia)-1] = '\0'; } 
        else { materia[0] = '\0'; }
        // ATUALIZADO: Coluna 12 (Turma)
        tok = TOK_R(NULL, ",", &saveptr);
        if (tok) { strncpy(turma, tok, sizeof(turma)-1); turma[sizeof(turma)-1] = '\0'; } 
        else { turma[0] = '\0'; }
        // --- Fim do Parse ---
        
        trimWhitespace(email);
        trimWhitespace(senha_do_arquivo); 

        if (strcmp(email_login, email) == 0 && strcmp(senha_login_hash, senha_do_arquivo) == 0) {
            encontrado = 1;
            break; 
        } 
    }
    fclose(arquivo);

    if (!encontrado) {
        printf("❌ Email ou senha incorretos.\n");
        return;
    }

    printf("\n✅ Bem-vindo, %s (%s)\n", nome, nivel);

    trimWhitespace(nivel);
    toLowerString(nivel);

    if (strcmp(nivel, "admin") == 0) {
        menuAdmin();
    }
    else if (strcmp(nivel, "coordenador") == 0) {
        menuCoordenador(); 
    }
    else if (strcmp(nivel, "professor") == 0) {
        menuProfessor();
    }
    else if (strcmp(nivel, "aluno") == 0) {
        // ATUALIZADO: Passa 'materia' e 'turma'
        menuAluno(nome, np1, np2, pim, materia, turma);
    }
    else {
        printf("ERRO: Nível de usuário '%s' desconhecido.\n", nivel);
    }
}

/**
 * @brief (ATUALIZADO) Inicializa o CSV com 12 colunas.
 */
void inicializarArquivoCSV() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "a+"); 
    if (!arquivo) {
        printf("ERRO FATAL: Não foi possível criar ou abrir o arquivo de usuários em %s.\n", NOME_ARQUIVO);
        printf("Verifique se você tem permissão de escrita no diretório atual.\n");
        exit(1);
    }
    
    fseek(arquivo, 0, SEEK_END);
    if (ftell(arquivo) == 0) {
        // ATUALIZADO: Cabeçalho com 12 colunas
        fprintf(arquivo, "ID,NOME,EMAIL,SENHA,IDADE,NIVEL,CURSO,NP1,NP2,PIM,MATERIA,TURMA\n");
    }
    fclose(arquivo);
}