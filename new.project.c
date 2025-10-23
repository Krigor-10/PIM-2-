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

#define NOME_ARQUIVO "C:\\estudos\\tabela_usuarios_geral.csv"
#define ARQUIVO_TURMAS "C:\\estudos\\tabela_turmas.csv" // NOVO ARQUIVO

// Tamanho máximo do nome da turma
#define TAM_NOME_TURMA 50

// =====================================================
// Função: limpar_buffer_entrada
// Remove resíduos no buffer de entrada (stdin)
// =====================================================
void limpar_buffer_entrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// =====================================================
// Função: obterMaiorID
// Lê o CSV e encontra o maior ID já existente
// =====================================================
int obterMaiorID() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) return 0;

    char linha[300];
    int maiorID = 0;

    // Pula cabeçalho
    fgets(linha, sizeof(linha), arquivo);

    while (fgets(linha, sizeof(linha), arquivo)) {
        int id;
        if (sscanf(linha, "%d,", &id) == 1) {
            if (id > maiorID) maiorID = id;
        }
    }

    fclose(arquivo);
    return maiorID;
}

// =====================================================
// Função: obterMaiorIDTurma (NOVA)
// =====================================================
int obterMaiorIDTurma() {
    FILE *arquivo = fopen(ARQUIVO_TURMAS, "r");
    if (!arquivo) return 0;

    char linha[100];
    int maiorID = 0;

    // Pula cabeçalho
    fgets(linha, sizeof(linha), arquivo);

    while (fgets(linha, sizeof(linha), arquivo)) {
        int id;
        // Espera ID,NOME_TURMA
        if (sscanf(linha, "%d,", &id) == 1) {
            if (id > maiorID) maiorID = id;
        }
    }

    fclose(arquivo);
    return maiorID;
}
// =====================================================

// =====================================================
// Funções de Gerenciamento de Turmas (NOVAS)
// =====================================================

// =====================================================
// Função: listarTurmas
// =====================================================
void listarTurmas() {
    FILE *arquivo = fopen(ARQUIVO_TURMAS, "r");
    if (!arquivo) {
        printf("\n⚠️ Não há turmas cadastradas. Crie uma primeiro.\n");
        return;
    }

    char linha[100];
    int idTurma;
    char nomeTurma[TAM_NOME_TURMA];
    int encontrado = 0;

    fgets(linha, sizeof(linha), arquivo); // Pula cabeçalho
    
    printf("\n=== TURMAS CADASTRADAS ===\n");
    printf("ID | NOME DA TURMA\n");
    printf("---|--------------\n");

    while (fgets(linha, sizeof(linha), arquivo)) {
        // ID,NOME_TURMA
        if (sscanf(linha, "%d,%49[^\n]", &idTurma, nomeTurma) == 2) {
            printf("%d | %s\n", idTurma, nomeTurma);
            encontrado = 1;
        }
    }

    if (!encontrado) {
        printf("Nenhuma turma encontrada no arquivo.\n");
    }

    fclose(arquivo);
}

void cadastrarUsuarioGeral(const char *nivelForcado) {
    FILE *arquivo;
    char nome[50], email[50], senha[20], nivel[50], curso[50];
    int idade, np1=0, np2=0, pim=0, idTurma=0; // idTurma é 0 por padrão

    arquivo = fopen(NOME_ARQUIVO, "a");
    if (arquivo == NULL) {
        printf("❌ Erro ao abrir o arquivo de usuários.\n");
        return;
    }

    int proximoID = obterMaiorID() + 1;
    strcpy(nivel, nivelForcado);

    printf("\n=== Cadastro de %s (ID %d) ===\n", nivelForcado, proximoID);

    printf("Nome: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = '\0';

    printf("Email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = '\0';

    printf("Senha: ");
    fgets(senha, sizeof(senha), stdin);
    senha[strcspn(senha, "\n")] = '\0';

    printf("Idade: ");
    scanf("%d", &idade);
    limpar_buffer_entrada();

    printf("Curso: ");
    fgets(curso, sizeof(curso), stdin);
    curso[strcspn(curso, "\n")] = '\0';

    if (strcmp(nivelForcado, "aluno") == 0) {
        listarTurmas();
        printf("ID da Turma (0 se nenhuma): ");
        scanf("%d", &idTurma);
        limpar_buffer_entrada();
        
        printf("NP1: ");
        scanf("%d", &np1);
        limpar_buffer_entrada();

        printf("NP2: ");
        scanf("%d", &np2);
        limpar_buffer_entrada();

        printf("PIM: ");
        scanf("%d", &pim);
        limpar_buffer_entrada();
    }
    
    // Formato: ID,NOME,EMAIL,SENHA,IDADE,NIVEL,CURSO,NP1,NP2,PIM,ID_TURMA
    fprintf(arquivo, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d,%d\n",
                    proximoID, nome, email, senha, idade, nivel, curso, np1, np2, pim, idTurma);

    fclose(arquivo);
    printf("\n✅ Usuário %s cadastrado e salvo com sucesso!\n", nome);
}
// =====================================================
// Função: cadastrarTurmas
// =====================================================
void cadastrarTurmas() {
    FILE *arquivo;
    char nomeTurma[TAM_NOME_TURMA];
    int quantidade;
    
    // Verifica se o arquivo existe para saber se deve escrever o cabeçalho
    int arquivoTemConteudo = 0;
    arquivo = fopen(ARQUIVO_TURMAS, "r");
    if (arquivo != NULL) {
        fseek(arquivo, 0, SEEK_END);
        long tamanho = ftell(arquivo);
        arquivoTemConteudo = (tamanho > 0);
        fclose(arquivo);
    }
    
    arquivo = fopen(ARQUIVO_TURMAS, "a");
    if (arquivo == NULL) {
        printf("❌ Erro ao abrir ou criar o arquivo de turmas.\n");
        return;
    }

    if (!arquivoTemConteudo) {
        fprintf(arquivo, "ID,NOME_TURMA\n");
    }

    printf("Quantas turmas deseja cadastrar? ");
    scanf("%d", &quantidade);
    limpar_buffer_entrada();
    
    int proximoID = obterMaiorIDTurma() + 1;

    for (int i = 0; i < quantidade; i++) {
        printf("\n=== Cadastro da Turma %d (ID %d) ===\n", i + 1, proximoID);

        printf("Nome da Turma (Ex: Eng. Produção 2024): ");
        fgets(nomeTurma, sizeof(nomeTurma), stdin);
        nomeTurma[strcspn(nomeTurma, "\n")] = '\0';

        fprintf(arquivo, "%d,%s\n", proximoID, nomeTurma);
        proximoID++;
    }

    fclose(arquivo);
    printf("\n✅ Turmas cadastradas com sucesso!\n");
}

// Função: cadastrarUsuarios
// =====================================================
// =====================================================
// Função: cadastrarUsuarios (Refatorada)
// Mantida para compatibilidade, agora pede qual nível cadastrar
// =====================================================
void cadastrarUsuarios() {
    int opcaoNivel;
    char nivel[50];
    
    printf("\nSelecione o nível que deseja cadastrar:\n");
    printf("1 - Admin\n2 - Coordenador\n3 - Professor\n4 - Aluno\n");
    printf("Opção: ");
    if (scanf("%d", &opcaoNivel) != 1) {
        limpar_buffer_entrada();
        printf("❌ Opção inválida.\n");
        return;
    }
    limpar_buffer_entrada();

    switch (opcaoNivel) {
        case 1: strcpy(nivel, "admin"); break;
        case 2: strcpy(nivel, "coordenador"); break;
        case 3: strcpy(nivel, "professor"); break;
        case 4: strcpy(nivel, "aluno"); break;
        default:
            printf("Nível inválido.\n");
            return;
    }
    
    // Chama a função geral de cadastro
    cadastrarUsuarioGeral(nivel);
}

void listarUsuariosPorNivel(const char *nivel_filtro) {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("❌ Não foi possível abrir o arquivo.\n");
        return;
    }

    char linha[300];
    int id, idade, np1, np2, pim, idTurma = 0;
    char nome[50], email[50], senha[20], nivel[50], curso[50];
    int encontrado = 0;

    fgets(linha, sizeof(linha), arquivo); // Pula cabeçalho
    printf("\n=== LISTA DE USUÁRIOS (%s) ===\n", nivel_filtro);
    
    printf("ID | NOME | IDADE | CURSO %s\n", (strcmp(nivel_filtro, "aluno") == 0) ? "| TURMA ID" : "");
    printf("---|------|-------|-------%s\n", (strcmp(nivel_filtro, "aluno") == 0) ? "|---------" : "");

    while (fgets(linha, sizeof(linha), arquivo)) {
        // Tenta ler 11 campos (novo formato com ID_TURMA)
        int camposLidos = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%49[^,],%49[^,],%d,%d,%d,%d",
                   &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, &idTurma);
        
        // Se falhar, tenta ler 10 campos (formato antigo)
        if (camposLidos < 10) continue;
        if (camposLidos == 10) idTurma = 0; // Se leu 10, a turma é 0

        nivel[strcspn(nivel, "\r\n")] = '\0';
        for (int i = 0; nivel[i]; i++) nivel[i] = tolower(nivel[i]);

        if (strcmp(nivel, nivel_filtro) == 0) {
            if (strcmp(nivel_filtro, "aluno") == 0) {
                printf("%d | %s | %d | %s | %d\n", id, nome, idade, curso, idTurma);
            } else {
                printf("%d | %s | %d | %s\n", id, nome, idade, curso);
            }
            encontrado = 1;
        }
    }
    
    if (!encontrado) {
        printf("Nenhum usuário '%s' encontrado.\n", nivel_filtro);
    }
    
    fclose(arquivo);
}

// =====================================================
// Função: atualizarNotas
// (Mantida, embora o menuProfessor tenha sua própria lógica)
// =====================================================
void atualizarNotas() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("❌ Arquivo não encontrado.\n");
        return;
    }

    FILE *temp = fopen("temp.csv", "w");
    if (!temp) {
        printf("❌ Erro ao criar arquivo temporário.\n");
        fclose(arquivo);
        return;
    }

    char linha[300];
    int id, idade, np1, np2, pim;
    char nome[50], email[50], senha[20], nivel[50], curso[50];
    int idBusca, encontrado = 0;

    // Copia cabeçalho
    fgets(linha, sizeof(linha), arquivo);
    fprintf(temp, "%s", linha);

    printf("\nDigite o ID do aluno que deseja atualizar: ");
    scanf("%d", &idBusca);
    limpar_buffer_entrada();

    while (fgets(linha, sizeof(linha), arquivo)) {
        sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%49[^,],%49[^,],%d,%d,%d",
               &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim);
        
        nivel[strcspn(nivel, "\r\n")] = '\0';
        for (int i = 0; nivel[i]; i++) nivel[i] = tolower(nivel[i]);

        if (id == idBusca && strcmp(nivel, "aluno") == 0) {
            printf("\nAluno encontrado: %s (%s)\n", nome, curso);
            printf("Notas atuais → NP1: %d | NP2: %d | PIM: %d\n", np1, np2, pim);

            printf("\nDigite a nova NP1: ");
            scanf("%d", &np1);
            limpar_buffer_entrada();

            printf("Digite a nova NP2: ");
            scanf("%d", &np2);
            limpar_buffer_entrada();

            printf("Digite a nova PIM: ");
            scanf("%d", &pim);
            limpar_buffer_entrada();

            encontrado = 1;
        }
        
        fprintf(temp, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d\n",
                id, nome, email, senha, idade, nivel, curso, np1, np2, pim);
    }

    fclose(arquivo);
    fclose(temp);

    if (encontrado) {
        remove(NOME_ARQUIVO);
        rename("temp.csv", NOME_ARQUIVO);
        printf("\n✅ Notas atualizadas com sucesso!\n");
    } else {
        remove("temp.csv");
        printf("\n⚠️ Aluno não encontrado ou não é do tipo 'aluno'.\n");
    }
}

// =====================================================
// Função: menuProfessor
// =====================================================
void menuProfessor() {
    int opcao;
    do {
        printf("\n====== MENU PROFESSOR ======\n");
        printf("1 - Atualizar notas de um aluno\n");
        printf("2 - Visualizar todos os alunos\n");
        printf("3 - Enviar exercícios\n");
        printf("0 - Sair\n");
        printf("Escolha uma opção: ");
        if (scanf("%d", &opcao) != 1) {
            limpar_buffer_entrada();
            opcao = -1;
        }
        limpar_buffer_entrada();

        if (opcao == 1) {
            // Lógica de atualização de notas
            FILE *arquivo = fopen(NOME_ARQUIVO, "r");
            if (!arquivo) {
                printf("❌ Erro: não foi possível abrir o arquivo.\n");
                continue;
            }

            FILE *temp = fopen("temp.csv", "w");
            if (!temp) {
                printf("❌ Erro: não foi possível criar arquivo temporário.\n");
                fclose(arquivo);
                continue;
            }

            char linha[300];
            int id_alvo, id, idade, np1, np2, pim;
            char nome[50], email[50], senha[20], nivel[50], curso[50];

            // Copia cabeçalho
            fgets(linha, sizeof(linha), arquivo);
            fprintf(temp, "%s", linha);

            printf("Digite o ID do aluno para atualizar notas: ");
            scanf("%d", &id_alvo);
            limpar_buffer_entrada();

            int encontrado = 0;

            while (fgets(linha, sizeof(linha), arquivo)) {
                sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%49[^,],%49[^,],%d,%d,%d",
                       &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim);

                nivel[strcspn(nivel, "\r\n")] = '\0';
                char nivel_copia[50];
                strcpy(nivel_copia, nivel);
                for (int i = 0; nivel_copia[i]; i++) nivel_copia[i] = tolower(nivel_copia[i]);

                if (id == id_alvo && strcmp(nivel_copia, "aluno") == 0) {
                    printf("Aluno encontrado: %s\n", nome);
                    printf("Nova NP1: "); scanf("%d", &np1); limpar_buffer_entrada();
                    printf("Nova NP2: "); scanf("%d", &np2); limpar_buffer_entrada();
                    printf("Novo PIM: "); scanf("%d", &pim); limpar_buffer_entrada();

                    encontrado = 1;
                }

                fprintf(temp, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d\n",
                        id, nome, email, senha, idade, nivel, curso, np1, np2, pim);
            }

            fclose(arquivo);
            fclose(temp);

            remove(NOME_ARQUIVO);
            rename("temp.csv", NOME_ARQUIVO);

            if (encontrado)
                printf("✅ Notas atualizadas com sucesso!\n");
            else
                printf("⚠️ Aluno não encontrado ou não é do tipo 'aluno'.\n");
        }

        else if (opcao == 2) {
            FILE *arquivo = fopen(NOME_ARQUIVO, "r");
            if (!arquivo) {
                printf("❌ Não foi possível abrir o arquivo.\n");
                continue;
            }

            char linha[300];
            int id, idade, np1, np2, pim;
            char nome[50], email[50], senha[20], nivel[50], curso[50];

            fgets(linha, sizeof(linha), arquivo); // cabeçalho
            printf("\n=== LISTA DE ALUNOS ===\n");
            while (fgets(linha, sizeof(linha), arquivo)) {
                sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%49[^,],%49[^,],%d,%d,%d",
                       &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim);

                nivel[strcspn(nivel, "\r\n")] = '\0';
                for (int i = 0; nivel[i]; i++) nivel[i] = tolower(nivel[i]);

                if (strcmp(nivel, "aluno") == 0) {
                    printf("ID: %d | Nome: %s | Curso: %s | NP1: %d | NP2: %d | PIM: %d\n",
                           id, nome, curso, np1, np2, pim);
                }
            }
            fclose(arquivo);
        }

        else if (opcao == 3) {
            char caminhoPDF[260];
            char pastaDestino[] = "C:\\estudos\\exercicios_enviados";
            FILE *arquivoOrigem;
            FILE *arquivoDestino;

            // Cria a pasta de destino, se não existir
            #ifdef _WIN32
                if (_mkdir(pastaDestino) != 0 && errno != EEXIST) {
                    printf("❌ Erro ao criar pasta de destino: %s. Verifique se o caminho existe.\n", pastaDestino);
                    continue;
                }
            #else
                if (mkdir(pastaDestino, 0777) != 0 && errno != EEXIST) {
                    printf("❌ Erro ao criar pasta de destino.\n");
                    continue;
                }
            #endif

            printf("\n--- Envio de Exercícios ---\n");
            printf("COMO SELECIONAR: \n");
            printf("1. Abra seu explorador de arquivos.\n");
            printf("2. ARRASTE e SOLTE o arquivo PDF nesta janela e pressione ENTER, OU digite o caminho completo.\n");
            printf("Caminho do arquivo PDF: ");
            
            // Leitura da entrada
            fgets(caminhoPDF, sizeof(caminhoPDF), stdin);
            caminhoPDF[strcspn(caminhoPDF, "\n")] = '\0'; // remove quebra de linha

            // =========================================================
            // CORREÇÃO CRÍTICA DO CAMINHO (Limpeza de Aspas e Espaços)
            // =========================================================
            size_t len = strlen(caminhoPDF);
            
            // 1. Remove aspas no início e no fim (comum ao arrastar e soltar)
            if (len > 1 && caminhoPDF[0] == '"' && caminhoPDF[len - 1] == '"') {
                memmove(caminhoPDF, caminhoPDF + 1, len - 2);
                caminhoPDF[len - 2] = '\0';
                len -= 2;
            }
            
            // 2. Remove espaços em branco no final (common no console)
            while (len > 0 && caminhoPDF[len - 1] == ' ') {
                caminhoPDF[--len] = '\0';
            }
            // =========================================================

            // 1. Verifica e abre o arquivo de origem (modo binário)
            // Nota: O fopen geralmente aceita '/' no Windows, mas requer a string limpa.
            arquivoOrigem = fopen(caminhoPDF, "rb");
            if (!arquivoOrigem) {
                // Tenta substituir barras invertidas por barras duplas para o Windows
                // Isso pode corrigir o problema de caracteres de escape se o usuário digitou o caminho
                char caminhoAlternativo[260];
                strcpy(caminhoAlternativo, caminhoPDF);
                for (char *p = caminhoAlternativo; *p; p++) {
                    if (*p == '\\') {
                        *p = '/'; // Tenta usar barra simples, mais universal
                    }
                }
                
                arquivoOrigem = fopen(caminhoAlternativo, "rb");

                if (!arquivoOrigem) {
                    printf("❌ Arquivo não encontrado ou caminho inválido.\n");
                    printf("Caminho fornecido: [%s]\n", caminhoPDF);
                    continue;
                }
            }
            // ... (o resto da lógica de cópia de arquivo permanece igual)
            
            // 2. Extrai o nome do arquivo (agora que o arquivo está aberto, o caminho é válido)
            // A extração usa o separador de caminho, que pode ser '\' ou '/'
            const char *nomeArquivo = strrchr(caminhoPDF, '\\'); // Tenta Windows
            if (!nomeArquivo) {
                nomeArquivo = strrchr(caminhoPDF, '/'); // Tenta Linux/universal
            }
            
            if (nomeArquivo) nomeArquivo++;
            else nomeArquivo = caminhoPDF; 
            
            if (strlen(nomeArquivo) == 0) {
                 printf("❌ Falha ao extrair o nome do arquivo.\n");
                 fclose(arquivoOrigem);
                 continue;
            }

            // 3. Monta o caminho de destino
            char destinoCompleto[400];
            snprintf(destinoCompleto, sizeof(destinoCompleto), "%s\\%s", pastaDestino, nomeArquivo);

            // 4. Abre o arquivo de destino (modo binário)
            arquivoDestino = fopen(destinoCompleto, "wb");
            // ... (resto da lógica de cópia e fechamento de arquivos)
            
            if (!arquivoDestino) {
                printf("❌ Erro ao criar arquivo de destino: %s\n", destinoCompleto);
                fclose(arquivoOrigem);
                continue;
            }
            
            // 5. Cópia do arquivo byte a byte
            char buffer[1024]; 
            size_t bytesLidos;

            while ((bytesLidos = fread(buffer, 1, sizeof(buffer), arquivoOrigem)) > 0) {
                if (fwrite(buffer, 1, bytesLidos, arquivoDestino) != bytesLidos) {
                    printf("❌ Erro de escrita no arquivo de destino.\n");
                    fclose(arquivoOrigem);
                    fclose(arquivoDestino);
                    continue;
                }
            }

            fclose(arquivoOrigem);
            fclose(arquivoDestino);
            
            printf("✅ PDF enviado com sucesso para: %s\n", destinoCompleto);
        }

    } while (opcao != 0);
}

// =====================================================
// Função: menuCoordenador (ATUALIZADA)
// =====================================================
void menuCoordenador() {
    int opcao;
    do {
        printf("\n====== MENU COORDENADOR ======\n");
        printf("1 - Cadastrar Turma\n");
        printf("2 - Listar Turmas\n");
        printf("3 - Cadastrar Professor\n");
        printf("4 - Cadastrar Aluno\n");
        printf("5 - Listar Professores\n");
        printf("6 - Listar Alunos\n");
        printf("7 - Listar Todos os Usuários (Relatório)\n");
        printf("0 - Sair\n");
        printf("Escolha uma opção: ");
        if (scanf("%d", &opcao) != 1) {
            limpar_buffer_entrada();
            opcao = -1;
        }
        limpar_buffer_entrada();

        switch (opcao) {
            case 1:
                cadastrarTurmas(); // NOVO
                break;
            case 2:
                listarTurmas(); // NOVO
                break;
            case 3:
                cadastrarUsuarioGeral("professor"); // Refatorado
                break;
            case 4:
                cadastrarUsuarioGeral("aluno"); // Refatorado
                break;
            case 5:
                listarUsuariosPorNivel("professor");
                break;
            case 6:
                listarUsuariosPorNivel("aluno");
                break;
            case 7:
                printf("\n--- Relatório de Usuários ---\n");
                listarUsuariosPorNivel("admin");
                listarUsuariosPorNivel("coordenador");
                listarUsuariosPorNivel("professor");
                listarUsuariosPorNivel("aluno");
                break;
            case 0:
                printf("Saindo do Menu Coordenador...\n");
                break;
            default:
                printf("Opção inválida.\n");
        }

    } while (opcao != 0);
}

// =====================================================
// Função: loginUsuario
// =====================================================
void loginUsuario() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("❌ Nenhum usuário cadastrado.\n");
        return;
    }

    char email_login[50], senha_login[20];
    char linha[300];
    char nome[50], email[50], senha[20], nivel[50], curso[50];
    int id, idade, np1=0, np2=0, pim=0, idTurma=0;
    int encontrado = 0;

    printf("\n=== LOGIN ===\n");
    printf("Email: ");
    fgets(email_login, sizeof(email_login), stdin);
    email_login[strcspn(email_login, "\n")] = '\0';

    printf("Senha: ");
    fgets(senha_login, sizeof(senha_login), stdin);
    senha_login[strcspn(senha_login, "\n")] = '\0';

    fgets(linha, sizeof(linha), arquivo); // pula cabeçalho

    while (fgets(linha, sizeof(linha), arquivo)) {
        // ATENÇÃO: sscanf agora tenta ler 11 campos (o último é o ID_TURMA)
        int camposLidos = sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%49[^,],%49[^,],%d,%d,%d,%d",
                   &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, &idTurma);
        
        if (camposLidos < 10) continue; // Linha mal formatada
        if (camposLidos == 10) idTurma = 0; // Para compatibilidade com linhas antigas sem ID_TURMA


        nivel[strcspn(nivel, "\r\n")] = '\0';
        char nivel_copia[50];
        strcpy(nivel_copia, nivel);
        for (int i = 0; nivel_copia[i]; i++) nivel_copia[i] = tolower(nivel_copia[i]); 

        if (strcmp(email_login, email) == 0 && strcmp(senha_login, senha) == 0) {
            encontrado = 1;
            strcpy(nivel, nivel_copia); 
            break;
        }
    }
    fclose(arquivo);

    if (!encontrado) {
        printf("❌ Email ou senha incorretos.\n");
        return;
    }

    printf("\n✅ Login bem-sucedido!\n");
    printf("Bem-vindo, %s (Nível: %s)\n", nome, nivel);

    if (strcmp(nivel, "professor") == 0) {
        // Professor agora pode ver a turma do aluno
        menuProfessor();
    } else if (strcmp(nivel, "aluno") == 0) {
        printf("📚 Suas notas → NP1: %d | NP2: %d | PIM: %d (Turma ID: %d)\n", np1, np2, pim, idTurma);
    } else if (strcmp(nivel, "admin") == 0) {
        printf("🔧 Acesso total ao sistema.\n");
    } else if (strcmp(nivel, "coordenador") == 0) {
        menuCoordenador();
    } else {
        printf("⚠️ Nível de acesso desconhecido.\n");
    }
}


// =====================================================
// Função principal
// =====================================================
int main() {
    int opcao;
    do {
        printf("\n========================\n");
        printf("📋 MENU PRINCIPAL\n");
        printf("========================\n");
        printf("1 - Cadastrar usuários\n");
        printf("2 - Fazer login\n");
        printf("0 - Sair\n");
        printf("Escolha uma opção: ");
        if (scanf("%d", &opcao) != 1) {
            limpar_buffer_entrada(); 
            opcao = -1;
        } else {
            limpar_buffer_entrada();
        }

        switch (opcao) {
            case 1: cadastrarUsuarios(); break;
            case 2: loginUsuario(); break;
            case 0: printf("Encerrando o programa...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (opcao != 0);

    return 0;
}