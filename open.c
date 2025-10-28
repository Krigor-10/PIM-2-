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

/* --- Bloco de Detecção de Plataforma Aprimorado ---
 * Define 'PLATAFORMA_WINDOWS' ou 'PLATAFORMA_POSIX' 
 * e inclui as bibliotecas corretas.
 */
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
void menuAdmin();
void menuCoordenador();
void menuProfessor();
void menuAluno(const char *nome, int np1, int np2, int pim, const char *turma);
void visualizarRelatorios();
void mostrarAlunosPorTurma();
void atualizarNotas();
void gerenciarTurmas();
void gerenciarUsuarios();
void excluirUsuario(const char *nivelExecutor);
void mostrarTodosUsuarios();
void cadastrarUsuarios();
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

// --- NOVAS FUNÇÕES (SENHA E HASH) ---

/**
 * @brief (NOVO) Lê uma senha do terminal sem mostrá-la, imprimindo '*' no lugar.
 * Suporta Windows (conio.h) e POSIX (termios.h).
 */
void obterSenhaOculta(char *senha, int tam_max) {
    int i = 0;
    char ch;

#ifdef PLATAFORMA_WINDOWS
    // --- Lógica para Windows ---
    while (1) {
        ch = _getch(); // Lê um caractere sem ecoar
        
        if (ch == '\r' || ch == '\n') { // ENTER
            senha[i] = '\0';
            printf("\n");
            break;
        } else if (ch == '\b') { // BACKSPACE
            if (i > 0) {
                i--;
                printf("\b \b"); // Apaga o '*' da tela
            }
        } else if (i < tam_max - 1 && isprint(ch)) { // Caractere normal
            senha[i] = ch;
            printf("*");
            i++;
        }
    }
#else
    // --- Lógica para Linux/macOS (POSIX) ---
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // Pega atributos atuais
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON); // Desliga eco e modo canônico
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Aplica novos atributos

    while (1) {
        ch = getchar();
        
        if (ch == '\n' || ch == '\r') { // ENTER
            senha[i] = '\0';
            printf("\n");
            break;
        } else if (ch == 127) { // BACKSPACE (ASCII 127)
            if (i > 0) {
                i--;
                printf("\b \b"); // Apaga o '*' da tela
            }
        } else if (i < tam_max - 1 && isprint(ch)) { // Caractere normal
            senha[i] = ch;
            printf("*");
            i++;
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restaura atributos originais
#endif
}

/**
 * @brief (NOVO) Gera um hash simples (djb2) para uma string.
 * Converte a senha original em um hash numérico (em hexadecimal).
 * @param senha_plain A senha em texto puro.
 * @param senha_hash_out O buffer onde o hash (string) será salvo.
 * @param tam_hash O tamanho do buffer de saída.
 */
void hashSenha(const char *senha_plain, char *senha_hash_out, int tam_hash) {
    unsigned long hash = 5381; // Valor inicial padrão do hash djb2
    int c;

    while ((c = *senha_plain++)) {
        // Lógica do hash: hash = hash * 33 + c
        hash = ((hash << 5) + hash) + c; 
    }
    
    // Converte o hash (um número longo) para uma string hexadecimal
    snprintf(senha_hash_out, tam_hash, "%lx", hash);
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


// --- CADASTRO DE USUÁRIOS (REFACHINADO E COM HASH) ---

/**
 * @brief Função base que REALIZA o cadastro.
 * Recebe o nível (admin, professor, aluno) como parâmetro.
 * (ATUALIZADO: Usa obterSenhaOculta e hashSenha)
 */
void cadastrarUsuarioBase(const char *nivel) {
    FILE *arquivo;
    // Buffers separados para senha pura e hash
    char nome[50], email[50], senha_plain[20], senha_hash[20], curso[50], turma[100];
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
    
    // --- MODIFICADO: Obter senha oculta ---
    printf("Senha: "); 
    obterSenhaOculta(senha_plain, sizeof(senha_plain)); 
    // --- FIM DA MODIFICAÇÃO ---
    
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

    // --- MODIFICADO: Gerar o HASH da senha ---
    hashSenha(senha_plain, senha_hash, sizeof(senha_hash));
    // --- FIM DA MODIFICAÇÃO ---

    // Salva no arquivo (SALVANDO O HASH)
    fprintf(arquivo, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d,%s\n",
            proximoID, nome, email, senha_hash, idade, nivel, curso, np1, np2, pim, turma);
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

/**
 * @brief Exclui um usuário do arquivo CSV pelo ID, aplicando regras de permissão.
 * @param nivelExecutor O nível do usuário que está executando a exclusão ("admin", "coordenador").
 */
void excluirUsuario(const char *nivelExecutor) {
    FILE *arquivo, *temp;
    char linha[400], linha_original[400];
    int idBusca, id, idade, np1, np2, pim;
    char nome[50], email[50], senha[20], nivel_alvo[20], curso[50], turma[100];
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

    // Copia o cabeçalho
    if (fgets(linha, sizeof(linha), arquivo)) {
        fprintf(temp, "%s", linha);
    }

    // Itera sobre o arquivo
    while (fgets(linha, sizeof(linha), arquivo)) {
        strcpy(linha_original, linha);

        int result = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%19[^,],%49[^,],%d,%d,%d,%99[^\n]",
                        &id, nome, email, senha, &idade, nivel_alvo, curso, &np1, &np2, &pim, turma);
        
        if (result != 11) {
            fprintf(temp, "%s", linha_original);
            continue;
        }

        if (id == idBusca) {
            encontrado = 1;
            trimWhitespace(nivel_alvo);
            toLowerString(nivel_alvo);

            int podeExcluir = 0; // Flag para controlar a permissão

            // --- LÓGICA DE PERMISSÃO ---
            if (strcmp(nivelExecutor, "admin") == 0) {
                if (strcmp(nivel_alvo, "admin") == 0) {
                    printf("❌ ERRO: Um Admin não pode excluir outro usuário 'admin'.\n");
                } else {
                    podeExcluir = 1; // Admin pode excluir qualquer outro nível
                }
            } 
            else if (strcmp(nivelExecutor, "coordenador") == 0) {
                if (strcmp(nivel_alvo, "professor") == 0 || strcmp(nivel_alvo, "aluno") == 0) {
                    podeExcluir = 1; // Coordenador pode excluir professor e aluno
                } else {
                    printf("❌ PERMISSÃO NEGADA: Coordenador não pode excluir usuários de nível '%s'.\n", nivel_alvo);
                }
            }

            // --- AÇÃO DE EXCLUSÃO ---
            if (podeExcluir) {
                printf("✅ Usuário '%s' (Nível: %s) foi excluído com sucesso.\n", nome, nivel_alvo);
                excluido = 1;
                // NÃO escreve a linha no arquivo temporário
            } else {
                // Se não tem permissão, mantém a linha no arquivo
                fprintf(temp, "%s", linha_original);
            }
        
        } else {
            // Se não é o ID procurado, apenas copia a linha para o temp
            fprintf(temp, "%s", linha_original);
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

    // Usa a macro de plataforma correta para criar o diretório
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
 * @brief (ATUALIZADO) Mostra todos os usuários com Média e Status
 */
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
        // --- CABEÇALHO ATUALIZADO ---
        printf("-------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        printf("| ID | NOME                   | EMAIL                  | NIVEL       | IDADE | CURSO                  | NP1 | NP2 | PIM | MÉDIA | STATUS    | TURMA                                   |\n");
        printf("-------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    }
    
    while (fgets(linha, sizeof(linha), arquivo)) {
        int result = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%19[^,],%49[^,],%d,%d,%d,%99[^\n]",
                            &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma);

        if (result == 11) {
            turma[strcspn(turma, "\n")] = '\0';
            
            // --- NOVA LÓGICA DE CÁLCULO DE STATUS/MÉDIA ---
            char media_str[10] = "N/A";
            char status_str[12] = "N/A";
            char nivel_temp[20];
            
            strcpy(nivel_temp, nivel);
            trimWhitespace(nivel_temp);
            toLowerString(nivel_temp);

            if (strcmp(nivel_temp, "aluno") == 0) {
                float media = (np1 * 4 + np2 * 4 + pim * 2) / 10.0;
                sprintf(media_str, "%.2f", media); // Formata a média como string

                if (media >= 7.0) {
                    strcpy(status_str, "APROVADO");
                } else {
                    strcpy(status_str, "REPROVADO");
                }
            }
            // --- FIM DA NOVA LÓGICA ---

            // --- PRINTF ATUALIZADO ---
            // Nota: O hash da senha NÃO é exibido.
            printf("| %-2d | %-20s | %-20s | %-11s | %-5d | %-20s | %-3d | %-3d | %-3d | %-5s | %-9s | %-36s|\n",
                   id, nome, email, nivel, idade, curso, np1, np2, pim, 
                   media_str, status_str, turma); 
            
        } else {
            linha[strcspn(linha, "\n")] = '\0'; 
            printf("| ERRO NA LEITURA DA LINHA (result=%d): %s\n", result, linha);
        }
    }
    
    printf("-------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    fclose(arquivo);
}


/**
 * @brief (ATUALIZADO) Gera um relatório de notas e a média geral para uma turma,
 * incluindo o STATUS de aprovação.
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
    // --- CABEÇALHO E LINHAS ATUALIZADOS ---
    printf("------------------------------------------------------------------------------\n");
    printf("| ID | NOME                   | NP1 | NP2 | PIM | MÉDIA | STATUS    |\n");
    printf("------------------------------------------------------------------------------\n");

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
            
            // --- NOVA LÓGICA DE STATUS ---
            const char *status;
            if (media_individual >= 7.0) {
                status = "APROVADO";
            } else {
                status = "REPROVADO";
            }
            // --- FIM DA NOVA LÓGICA ---
            
            // --- PRINTF ATUALIZADO ---
            printf("| %-2d | %-20s | %-3d | %-3d | %-3d | %-5.2f | %-9s |\n",
                   id, nome, np1, np2, pim, media_individual, status);
        }
    }

    printf("------------------------------------------------------------------------------\n");

    if (alunos_encontrados == 0) {
        printf("Nenhum aluno encontrado para a turma '%s'.\n", turma_escolhida);
    } else {
        float media_geral_turma = soma_medias_turma / alunos_encontrados;
        char texto_media[100];
        sprintf(texto_media, "Média Geral da Turma: %.2f", media_geral_turma);

        // --- CAMPO DE MÉDIA ATUALIZADO ---
        printf("| %-76s |\n", texto_media);
        printf("------------------------------------------------------------------------------\n");
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
    printf("| ID | NOME                   | EMAIL                                | CURSO                  |\n");
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
        printf("3 - Excluir usuário\n");
        printf("0 - Voltar ao Menu Admin\n");
        printf("Opção: ");
        
        if(scanf("%d", &op) != 1) op = -1;
        limpar_buffer_entrada();

        switch (op) {
            case 1: mostrarTodosUsuarios(); break;
            case 2: cadastrarUsuarios(); break; 
            case 3: excluirUsuario("admin"); break; // Passa o nível "admin"
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
        printf("1 - Cadastrar (Professor ou Aluno)\n");
        printf("2 - Atualizar Notas de Aluno\n");
        printf("3 - Visualizar Relatórios (Notas por Turma)\n");
        printf("4 - Mostrar Alunos por Turma\n");
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
                mostrarAlunosPorTurma();
                break;
            case 5:
                excluirUsuario("coordenador"); // Passa o nível "coordenador"
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
 * @brief (ATUALIZADO) Menu do Aluno, agora mostra o Status.
 */
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
            // --- CASE 1 ATUALIZADO ---
            case 1: {
                float media = (np1 * 4 + np2 * 4 + pim * 2) / 10.0;
                
                // --- NOVA LÓGICA DE STATUS ---
                const char *status;
                if (media >= 7.0) {
                    status = "APROVADO";
                } else {
                    status = "REPROVADO";
                }
                // --- FIM DA NOVA LÓGICA ---
                
                // --- EXIBIÇÃO MELHORADA ---
                printf("\n--- Minhas Notas ---\n");
                printf("NP1: %d\n", np1);
                printf("NP2: %d\n", np2);
                printf("PIM: %d\n", pim);
                printf("--------------------\n");
                printf("Média Final: %.2f\n", media);
                printf("Status: %s\n", status); // <-- ADICIONADO
                printf("--------------------\n");
                break;
            }
            // --- FIM DA ATUALIZAÇÃO ---
            case 2:
                printf("\n🎥 Acessando vídeo-aula da turma: %s\n", turma);
                // Gera uma URL "falsa" do YouTube para a turma
                printf("URL: https://www.youtube.com/results?search_query=%s\n", turma);
                break;
            case 0: printf("Saindo do menu aluno...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

/**
 * @brief (ATUALIZADO) Função de Login
 * Agora usa obterSenhaOculta() e compara o HASH da senha digitada
 * com o HASH salvo no arquivo.
 */
void loginUsuario() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Nenhum usuário cadastrado.\n");
        return;
    }

    // Buffers para senha pura (digitada) e hash (gerado e lido)
    char email_login[50], senha_login_plain[20], senha_login_hash[20];
    char linha[400];
    char nome[50], email[50], senha_do_arquivo[20], nivel[20], curso[50], turma[100];
    int id, idade, np1, np2, pim;
    int encontrado = 0;

    printf("\nEmail: "); fgets(email_login, sizeof(email_login), stdin);
    email_login[strcspn(email_login, "\n")] = '\0';
    
    // --- MODIFICADO: Obter senha oculta ---
    printf("Senha: "); 
    obterSenhaOculta(senha_login_plain, sizeof(senha_login_plain));
    // --- FIM DA MODIFICAÇÃO ---

    // Normaliza a entrada do usuário
    trimWhitespace(email_login);

    // --- MODIFICADO: Gera o HASH da senha que o usuário DIGITOU ---
    hashSenha(senha_login_plain, senha_login_hash, sizeof(senha_login_hash));
    // --- FIM DA MODIFICAÇÃO ---

    fgets(linha, sizeof(linha), arquivo); // pula cabeçalho
    while (fgets(linha, sizeof(linha), arquivo)) {
        
        // Lê o hash salvo na coluna 'senha_do_arquivo'
        int result = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%19[^,],%49[^,],%d,%d,%d,%99[^\n]",
                  &id, nome, email, senha_do_arquivo, &idade, nivel, curso, &np1, &np2, &pim, turma);

        if (result != 11) {
           continue;
        }

        // Normaliza dados lidos do arquivo
        trimWhitespace(email);
        trimWhitespace(senha_do_arquivo); // Limpa o hash lido

        // --- MODIFICADO: Compara HASH com HASH ---
        // Compara o email digitado com o email do arquivo
        // E compara o HASH da senha digitada com o HASH salvo no arquivo
        if (strcmp(email_login, email) == 0 && strcmp(senha_login_hash, senha_do_arquivo) == 0) {
            encontrado = 1;
            turma[strcspn(turma, "\n")] = '\0';
            break; 
        } 
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
        printf("Verifique se você tem permissão de escrita no diretório atual.\n");
        exit(1);
    }
    
    fseek(arquivo, 0, SEEK_END);
    if (ftell(arquivo) == 0) {
        // Cabeçalho do CSV
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
            case 1: cadastrarUsuarios(); break; 
            case 2: loginUsuario(); break;
            case 0: printf("Encerrando...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (opc != 0);
    return 0;
}