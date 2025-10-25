#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

// --- CONSTANTES GLOBAIS ---
#define NOME_ARQUIVO "C:\\estudos\\tabela_usuarios.csv"
#define PASTA_EXERCICIOS "C:\\estudos\\exercicios_enviados"
#define TAM_TURMA 8

// Lista de turmas fixas
const char *TURMAS[TAM_TURMA] = {
    "ENGENHARIA DE SOFTWARE AGIL",
    "ALGORITIMO E ESTRUTURA DE DADOS PYTHON",
    "PROGRAMACAO ESTRUTURADA EM C",
    "ANALISE E PROJETO DE SISTEMAS",
    "PESQUISA TECNOLOGIA E INOVACAO",
    "EDUCACAO AMBIENTAL",
    "REDES DE COMPUTADORES",
    "INTELIGENCIA ARTIFICIAL"
};


// --- DECLARAÇÕES DE FUNÇÕES (PROTÓTIPOS) ---
// (Lista de protótipos ATUALIZADA)
void menuAdmin();
void menuCoordenador();
void menuProfessor();
void menuAluno(const char *nome, int np1, int np2, int pim, const char *turma);
void visualizarRelatorios();
void mostrarAlunosPorTurma();      // <-- NOVO
void atualizarNotas();
void gerenciarTurmas();
void gerenciarUsuarios();
void mostrarTodosUsuarios();
void cadastrarUsuarios();
void cadastrarUsuariosCoordenador(); // <-- NOVO
void cadastrarUsuarioBase(const char *nivel); // <-- NOVO (Refatorado)
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


// --- FUNÇÕES PRINCIPAIS DO SISTEMA ---

// Obter maior ID
int obterMaiorID() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) return 0;

    char linha[300];
    int maiorID = 0, id;
    fgets(linha, sizeof(linha), arquivo); // Pula cabeçalho
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (sscanf(linha, "%d,", &id) == 1)
            if (id > maiorID) maiorID = id;
    }
    fclose(arquivo);
    return maiorID;
}

/**
 * @brief Verifica se um usuário com nível "admin" já existe no arquivo.
 */
int adminExiste() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) return 0; 

    char linha[400];
    char nivel[20];
    // Buffers para os outros campos (necessários para o sscanf)
    char nome[50], email[50], senha[20], curso[50], turma[100];
    int id, idade, np1, np2, pim;
    int adminEncontrado = 0;

    fgets(linha, sizeof(linha), arquivo); // Pula cabeçalho

    while (fgets(linha, sizeof(linha), arquivo)) {
        
        int result = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%19[^,],%49[^,],%d,%d,%d,%99[^\n]",
              &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma);

        if (result != 11) continue; 

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


// --- CADASTRO DE USUÁRIOS (REFACHINADO) ---

/**
 * @brief Função base que REALIZA o cadastro.
 * Recebe o nível (admin, professor, aluno) como parâmetro.
 */
void cadastrarUsuarioBase(const char *nivel) {
    FILE *arquivo;
    char nome[50], email[50], senha[20], curso[50], turma[100];
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
    printf("Senha: "); fgets(senha, sizeof(senha), stdin); senha[strcspn(senha, "\n")] = '\0';
    printf("Idade: ");
    if (scanf("%d", &idade) != 1) {
        printf("Idade inválida.\n");
        limpar_buffer_entrada();
        fclose(arquivo);
        return;
    }
    limpar_buffer_entrada();
    printf("Curso: "); fgets(curso, sizeof(curso), stdin); curso[strcspn(curso, "\n")] = '\0';

    // Limpa os dados
    trimWhitespace(nome);
    trimWhitespace(email);
    trimWhitespace(senha);
    trimWhitespace(curso);

    // Garante que o curso não fique vazio (causa do bug do sscanf)
    if (strlen(curso) == 0) {
        strcpy(curso, "-");
    }

    // A lógica de turma só se aplica se o nível for "aluno"
    if (strcmp(nivel, "aluno") == 0) {
        printf("\nEscolha a turma:\n");
        for (int i = 0; i < TAM_TURMA; i++) {
            printf("%d - %s\n", i + 1, TURMAS[i]);
        }
        int opc;
        printf("Opção: ");
        if (scanf("%d", &opc) != 1) {
            limpar_buffer_entrada();
            strcpy(turma, "SEM TURMA");
        } else {
            limpar_buffer_entrada();
            if (opc >= 1 && opc <= TAM_TURMA) {
                strcpy(turma, TURMAS[opc - 1]);
            } else {
                strcpy(turma, "SEM TURMA");
            }
        }
    } else {
        // Professores, Admins, etc.
        strcpy(turma, "-");
    }

    // Salva no arquivo
    fprintf(arquivo, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d,%s\n",
            proximoID, nome, email, senha, idade, nivel, curso, np1, np2, pim, turma);
    fclose(arquivo);
    printf("\n✅ Usuário cadastrado com sucesso!\n");
}

/**
 * @brief Função de cadastro para (Admin / Menu Principal).
 * Pergunta o nível e impede o cadastro de mais de um "admin".
 */
void cadastrarUsuarios() {
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
            // VERIFICAÇÃO DE ADMIN ÚNICO
            if (adminExiste()) {
                printf("\n❌ Erro: Já existe um usuário 'admin' no sistema.\n");
                printf("Não é possível cadastrar mais de um administrador.\n");
                return; // Aborta o cadastro
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

    // Chama a função de cadastro real passando o nível escolhido
    cadastrarUsuarioBase(nivel);
}


/**
 * @brief (NOVO) Menu para o Coordenador cadastrar (Professor ou Aluno).
 * Limita a escolha apenas a "professor" ou "aluno".
 */
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
            return; // Sai sem cadastrar
        default:
            printf("Opção inválida.\n");
            return; // Sai sem cadastrar
    }
    
    // Chama a função base com o nível restrito
    cadastrarUsuarioBase(nivel);
}

// --- FIM DA SEÇÃO DE CADASTRO ---


// Atualizar notas
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
    char nome[50], email[50], senha[20], nivel[20], curso[50], turma[100];
    int encontrado = 0;

    if(fgets(linha, sizeof(linha), arquivo) == NULL) {
         printf("Arquivo vazio.\n");
         fclose(arquivo);
         fclose(temp);
         remove("temp.csv");
         return;
    }
    fprintf(temp, "%s", linha);

    printf("Digite o ID do aluno: ");
    if(scanf("%d", &idBusca) != 1) {
        printf("ID inválido.\n");
        limpar_buffer_entrada();
        fclose(arquivo);
        fclose(temp);
        remove("temp.csv");
        return;
    }
    limpar_buffer_entrada();

    while (fgets(linha, sizeof(linha), arquivo)) {
        int result = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%19[^,],%49[^,],%d,%d,%d,%99[^\n]",
              &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma);

        if (result != 11) {
            fprintf(temp, "%s", linha); 
            continue; 
        }

        trimWhitespace(nivel);
        toLowerString(nivel);
        
        if (id == idBusca && strcmp(nivel, "aluno") == 0) {
            printf("Aluno encontrado: %s\n", nome);
            printf("Nova NP1 (Atual: %d): ", np1); scanf("%d", &np1);
            printf("Nova NP2 (Atual: %d): ", np2); scanf("%d", &np2);
            printf("Novo PIM (Atual: %d): ", pim); scanf("%d", &pim);
            limpar_buffer_entrada();
            encontrado = 1;
        }

        turma[strcspn(turma, "\n")] = '\0';
        
        fprintf(temp, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d,%s\n",
                 id, nome, email, senha, idade, nivel, curso, np1, np2, pim, turma);
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

// Enviar exercício (PDF)
void enviarExercicioPDF() {
    char caminhoPDF[260], destino[300], nomeArquivo[100];
    FILE *origem, *destinoF;

    #ifdef _WIN32
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

    snprintf(destino, sizeof(destino), "%s\\%s", PASTA_EXERCICIOS, nomeArquivo);

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

// Mostrar todos os usuários
void mostrarTodosUsuarios() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Nenhum usuário cadastrado.\n");
        return;
    }

    char linha[400];
    int id, idade, np1, np2, pim;
    char nome[50], email[50], senha[20], nivel[20], curso[50], turma[100];

    printf("\n=== USUÁRIOS CADASTRADOS ===\n");
    
    if (fgets(linha, sizeof(linha), arquivo)) {
        printf("------------------------------------------------------------------------------------------------------------------------------------------------\n");
        printf("| ID | NOME                 | EMAIL                | NIVEL       | IDADE | CURSO                | NP1 | NP2 | PIM | TURMA                                |\n");
        printf("------------------------------------------------------------------------------------------------------------------------------------------------\n");
    }
    
    while (fgets(linha, sizeof(linha), arquivo)) {
        int result = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%19[^,],%49[^,],%d,%d,%d,%99[^\n]",
                            &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma);

        if (result == 11) {
            turma[strcspn(turma, "\n")] = '\0';
            printf("| %-2d | %-20s | %-20s | %-11s | %-5d | %-20s | %-3d | %-3d | %-3d | %-36s|\n",
                   id, nome, email, nivel, idade, curso, np1, np2, pim, turma);
        } else {
            linha[strcspn(linha, "\n")] = '\0'; 
            printf("| ERRO NA LEITURA DA LINHA (result=%d): %s\n", result, linha);
        }
    }
    
    printf("------------------------------------------------------------------------------------------------------------------------------------------------\n");
    fclose(arquivo);
}

/**
 * @brief Gera um relatório de notas e a média geral para uma turma.
 */
void visualizarRelatorios() {
    char turma_escolhida[100];
    int opc;

    printf("\n=== RELATÓRIO DE NOTAS POR TURMA ===\n");
    printf("Escolha a turma para gerar o relatório:\n");
    
    for (int i = 0; i < TAM_TURMA; i++) {
        printf("%d - %s\n", i + 1, TURMAS[i]);
    }
    printf("Opção: ");

    if (scanf("%d", &opc) != 1) {
        limpar_buffer_entrada(); 
        printf("Erro: Entrada inválida. Por favor, digite um número.\n");
        return;
    }
    limpar_buffer_entrada(); 

    if (opc < 1 || opc > TAM_TURMA) {
        printf("Opção inválida.\n");
        return;
    }
    
    strcpy(turma_escolhida, TURMAS[opc - 1]); 

    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo de usuários.\n");
        return;
    }

    char linha[400];
    char nome[50], email[50], senha[20], nivel[20], curso[50], turma_lida[100];
    int id, idade, np1, np2, pim;
    int alunos_encontrados = 0;
    float soma_medias_turma = 0.0;

    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        printf("Arquivo de usuários está vazio ou ilegível.\n");
        fclose(arquivo);
        return;
    }

    printf("\n--- Relatório de Notas: %s ---\n", turma_escolhida);
    printf("------------------------------------------------------------------\n");
    printf("| ID | NOME                 | NP1 | NP2 | PIM | MÉDIA |\n");
    printf("------------------------------------------------------------------\n");

    while (fgets(linha, sizeof(linha), arquivo)) {
        
        int result = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%19[^,],%49[^,],%d,%d,%d,%99[^\n]",
              &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma_lida);
        
        if (result != 11) continue; 

        turma_lida[strcspn(turma_lida, "\n")] = '\0';
        trimWhitespace(nivel);
        toLowerString(nivel);
        trimWhitespace(turma_lida);
        toUpperString(turma_lida); 
        
        if (strcmp(nivel, "aluno") == 0 && strcmp(turma_lida, turma_escolhida) == 0) {
            alunos_encontrados++;
            float media_individual = (np1 * 4 + np2 * 4 + pim * 2) / 10.0;
            soma_medias_turma += media_individual;
            
            printf("| %-2d | %-20s | %-3d | %-3d | %-3d | %-5.2f |\n", 
                   id, nome, np1, np2, pim, media_individual);
        }
    }

    printf("------------------------------------------------------------------\n");

    if (alunos_encontrados == 0) {
        printf("Nenhum aluno encontrado para a turma '%s'.\n", turma_escolhida);
    } else {
        float media_geral_turma = soma_medias_turma / alunos_encontrados;
        char texto_media[100];
        sprintf(texto_media, "Média Geral da Turma: %.2f", media_geral_turma);

        printf("| %-64s |\n", texto_media);
        printf("------------------------------------------------------------------\n");
        printf("Total: %d aluno(s) encontrado(s).\n", alunos_encontrados);
    }
    
    fclose(arquivo);
    
    printf("\nPressione ENTER para voltar ao menu...");
    getchar(); 
}

/**
 * @brief (NOVO) Lista todos os alunos (ID, Nome, Email, Curso) de uma turma específica.
 */
void mostrarAlunosPorTurma() {
    char turma_escolhida[100];
    int opc;

    printf("\n=== MOSTRAR ALUNOS POR TURMA ===\n");
    printf("Escolha a turma para listar os alunos:\n");
    
    for (int i = 0; i < TAM_TURMA; i++) {
        printf("%d - %s\n", i + 1, TURMAS[i]);
    }
    printf("Opção: ");

    if (scanf("%d", &opc) != 1) {
        limpar_buffer_entrada(); 
        printf("Erro: Entrada inválida. Por favor, digite um número.\n");
        return;
    }
    limpar_buffer_entrada(); 

    if (opc < 1 || opc > TAM_TURMA) {
        printf("Opção inválida.\n");
        return;
    }
    
    strcpy(turma_escolhida, TURMAS[opc - 1]); 

    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo de usuários.\n");
        return;
    }

    char linha[400];
    char nome[50], email[50], senha[20], nivel[20], curso[50], turma_lida[100];
    int id, idade, np1, np2, pim; 
    int alunos_encontrados = 0;

    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        printf("Arquivo de usuários está vazio ou ilegível.\n");
        fclose(arquivo);
        return;
    }

    printf("\n--- Alunos da Turma: %s ---\n", turma_escolhida);
    printf("----------------------------------------------------------------------------------\n");
    printf("| ID | NOME                 | EMAIL                          | CURSO                |\n");
    printf("----------------------------------------------------------------------------------\n");

    while (fgets(linha, sizeof(linha), arquivo)) {
        
        int result = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%19[^,],%49[^,],%d,%d,%d,%99[^\n]",
              &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma_lida);
        
        if (result != 11) continue; 

        turma_lida[strcspn(turma_lida, "\n")] = '\0';
        trimWhitespace(nivel);
        toLowerString(nivel);
        trimWhitespace(turma_lida);
        toUpperString(turma_lida); 
        
        if (strcmp(nivel, "aluno") == 0 && strcmp(turma_lida, turma_escolhida) == 0) {
            alunos_encontrados++;
            
            printf("| %-2d | %-20s | %-30s | %-20s |\n", 
                   id, nome, email, curso);
        }
    }

    printf("----------------------------------------------------------------------------------\n");
    if (alunos_encontrados == 0) {
        printf("Nenhum aluno encontrado para a turma '%s'.\n", turma_escolhida);
    } else {
        printf("Total: %d aluno(s) encontrado(s).\n", alunos_encontrados);
    }
    
    fclose(arquivo);
    
    printf("\nPressione ENTER para voltar ao menu...");
    getchar(); 
}


// --- MENUS E GERENCIAMENTO ---

// Gerenciar usuários (Submenu do Admin)
void gerenciarUsuarios() {
    int op;
    do {
        printf("\n=== GERENCIAR USUÁRIOS ===\n");
        printf("1 - Mostrar todos os usuários\n");
        printf("2 - Cadastrar novo usuário\n");
        printf("0 - Voltar ao Menu Admin\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1; // Trata entrada inválida
        limpar_buffer_entrada();

        switch (op) {
            case 1: mostrarTodosUsuarios(); break;
            case 2: cadastrarUsuarios(); break; // Admin chama a função de cadastro completa
            case 0: break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

// Gerenciar turmas (Apenas lista)
void gerenciarTurmas() {
    printf("\n=== TURMAS DO SISTEMA ===\n");
    for (int i = 0; i < TAM_TURMA; i++) {
        printf("%d - %s\n", i + 1, TURMAS[i]);
    }
    printf("===========================\n");
    
    printf("Pressione ENTER para voltar...");
    getchar(); 
}

// Menu Admin
void menuAdmin() {
    int op;
    do {
        printf("\n=== MENU ADMIN ===\n");
        printf("1 - Gerenciar todos usuários (Coordenador, Professor, Aluno)\n");
        printf("2 - Gerenciar turmas\n");
        printf("3 - Atualizar notas de aluno\n");
        printf("0 - Sair\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1;
        limpar_buffer_entrada();

        switch (op) {
            case 1: gerenciarUsuarios(); break;
            case 2: gerenciarTurmas(); break;
            case 3: atualizarNotas(); break;
            case 0: printf("Saindo do menu admin...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

// Menu Professor
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

// Menu Coordenador (ATUALIZADO)
void menuCoordenador() {
    int op;
    do {
        printf("\n=== MENU COORDENADOR ===\n");
        printf("1 - Cadastrar (Professor ou Aluno)\n"); // <-- NOVO
        printf("2 - Atualizar Notas de Aluno\n");
        printf("3 - Visualizar Relatórios (Notas por Turma)\n");
        printf("4 - Mostrar Alunos por Turma\n"); // <-- NOVO
        printf("0 - Sair\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1; // Trata entrada inválida
        limpar_buffer_entrada(); 

        switch (op) {
            case 1:
                cadastrarUsuariosCoordenador(); // <-- NOVO
                break;
            case 2:
                atualizarNotas();
                break;
            case 3:
                visualizarRelatorios();
                break;
            case 4:
                mostrarAlunosPorTurma(); // <-- NOVO
                break;
            case 0:
                printf("Saindo do menu coordenador...\n");
                break;
            default:
                printf("Opção inválida.\n");
        }
    } while (op != 0);
}

// Menu Aluno
void menuAluno(const char *nome, int np1, int np2, int pim, const char *turma) {
    int op;
    do {
        printf("\n=== MENU ALUNO (%s) ===\n", nome);
        printf("1 - Ver notas e média\n");
        printf("2 - Acessar vídeo-aula da turma\n");
        printf("0 - Sair\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1;
        limpar_buffer_entrada();

        switch (op) {
            case 1: {
                float media = (np1 * 4 + np2 * 4 + pim * 2) / 10.0;
                printf("\nNotas → NP1: %d | NP2: %d | PIM: %d | Média Final: %.2f\n", np1, np2, pim, media);
                break;
            }
            case 2:
                printf("\n🎥 Acessando vídeo-aula da turma: %s\n", turma);
                printf("URL: https://www.youtube.com/results?search_query=%s\n", turma);
                break;
            case 0: printf("Saindo do menu aluno...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

// Login
void loginUsuario() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Nenhum usuário cadastrado.\n");
        return;
    }

    char email_login[50], senha_login[20];
    char linha[400];
    char nome[50], email[50], senha[20], nivel[20], curso[50], turma[100];
    int id, idade, np1, np2, pim;
    int encontrado = 0;

    printf("\nEmail: "); fgets(email_login, sizeof(email_login), stdin);
    email_login[strcspn(email_login, "\n")] = '\0';
    printf("Senha: "); fgets(senha_login, sizeof(senha_login), stdin);
    senha_login[strcspn(senha_login, "\n")] = '\0';

    // Normaliza a entrada do usuário
    trimWhitespace(email_login);
    trimWhitespace(senha_login);

    // ----- DEBUG (Pode ser removido após testes) -----
    // printf("[DEBUG] Tentando login com Email: '%s' | Senha: '%s'\n", email_login, senha_login);
    // --------------------------------------------------

    fgets(linha, sizeof(linha), arquivo); // pula cabeçalho
    while (fgets(linha, sizeof(linha), arquivo)) {
        
        // Máscara sscanf SEM ESPAÇOS
        int result = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%19[^,],%49[^,],%d,%d,%d,%99[^\n]",
              &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma);

        if (result != 11) {
             // Linha mal formatada
             // printf("[DEBUG] Falha no sscanf (result=%d). Linha ignorada: %s", result, linha);
            continue;
        }

        // Normaliza dados lidos do arquivo
        trimWhitespace(email);
        trimWhitespace(senha);

        // Comparação final
        if (strcmp(email_login, email) == 0 && strcmp(senha_login, senha) == 0) {
            encontrado = 1;
            turma[strcspn(turma, "\n")] = '\0';
            break; 
        } 
        // (Debug de senha errada removido)
    }
    fclose(arquivo);

    if (!encontrado) {
        printf("❌ Email ou senha incorretos.\n");
        return;
    }

    printf("\n✅ Bem-vindo, %s (%s)\n", nome, nivel);

    // Normaliza a string 'nivel' para garantir redirecionamento correto
    trimWhitespace(nivel);
    toLowerString(nivel);

    // Bloco de redirecionamento
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
        menuAluno(nome, np1, np2, pim, turma);
    }
    else {
        printf("ERRO: Nível de usuário '%s' desconhecido.\n", nivel);
    }
}

// Inicialização do arquivo CSV
void inicializarArquivoCSV() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "a+"); 
    if (!arquivo) {
        printf("ERRO FATAL: Não foi possível criar ou abrir o arquivo de usuários em %s.\n", NOME_ARQUIVO);
        printf("Verifique se a pasta C:\\estudos existe.\n");
        exit(1);
    }
    
    fseek(arquivo, 0, SEEK_END);
    if (ftell(arquivo) == 0) {
        fprintf(arquivo, "ID,NOME,EMAIL,SENHA,IDADE,NIVEL,CURSO,NP1,NP2,PIM,TURMA\n");
    }
    fclose(arquivo);
}

// --- FUNÇÃO PRINCIPAL ---
int main() {
    inicializarArquivoCSV(); 
    
    int opc;
    do {
        printf("\n===== SISTEMA ACADÊMICO =====\n");
        printf("1 - Cadastrar usuário\n");
        printf("2 - Login\n");
        printf("0 - Sair\n");
        printf("Opção: ");
        
        if(scanf("%d", &opc) != 1) opc = -1; // Trata entrada inválida
        limpar_buffer_entrada();

        switch (opc) {
            case 1: cadastrarUsuarios(); break; // Chama a função de cadastro completa
            case 2: loginUsuario(); break;
            case 0: printf("Encerrando...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (opc != 0);
    return 0;
}