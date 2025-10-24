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

void limpar_buffer_entrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Obter maior ID
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

// Cadastro de usuários
void cadastrarUsuarios() {
    FILE *arquivo;
    char nome[50], email[50], senha[20], nivel[20], curso[50], turma[100];
    int idade, np1 = 0, np2 = 0, pim = 0;
    int opcaoNivel;

    printf("\nSelecione o nível de acesso:\n");
    printf("1 - Admin\n2 - Coordenador\n3 - Professor\n4 - Aluno\n");
    printf("Opção: ");
    scanf("%d", &opcaoNivel);
    limpar_buffer_entrada();

    switch (opcaoNivel) {
        case 1: strcpy(nivel, "admin"); break;
        case 2: strcpy(nivel, "coordenador"); break;
        case 3: strcpy(nivel, "professor"); break;
        case 4: strcpy(nivel, "aluno"); break;
        default: printf("Opção inválida.\n"); return;
    }

    int proximoID = obterMaiorID() + 1;
    // O arquivo é aberto em "a" pois o cabeçalho foi escrito na inicialização
    arquivo = fopen(NOME_ARQUIVO, "a");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo para escrita.\n");
        return;
    }

    printf("\n=== Cadastro de %s ===\n", nivel);
    printf("Nome: "); fgets(nome, sizeof(nome), stdin); nome[strcspn(nome, "\n")] = '\0';
    printf("Email: "); fgets(email, sizeof(email), stdin); email[strcspn(email, "\n")] = '\0';
    printf("Senha: "); fgets(senha, sizeof(senha), stdin); senha[strcspn(senha, "\n")] = '\0';
    printf("Idade: "); scanf("%d", &idade); limpar_buffer_entrada();
    printf("Curso: "); fgets(curso, sizeof(curso), stdin); curso[strcspn(curso, "\n")] = '\0';

    if (strcmp(nivel, "aluno") == 0) {
        printf("\nEscolha a turma:\n");
        for (int i = 0; i < TAM_TURMA; i++)
            printf("%d - %s\n", i + 1, TURMAS[i]);
        int opc;
        printf("Opção: ");
        scanf("%d", &opc);
        limpar_buffer_entrada();
        if (opc >= 1 && opc <= TAM_TURMA)
            strcpy(turma, TURMAS[opc - 1]);
        else
            strcpy(turma, "SEM TURMA");
    } else {
        strcpy(turma, "-");
    }

    fprintf(arquivo, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d,%s\n",
             proximoID, nome, email, senha, idade, nivel, curso, np1, np2, pim, turma);
    fclose(arquivo);
    printf("\n✅ Usuário cadastrado com sucesso!\n");
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

    fgets(linha, sizeof(linha), arquivo);
    fprintf(temp, "%s", linha);

    printf("Digite o ID do aluno: ");
    scanf("%d", &idBusca);
    limpar_buffer_entrada();

    while (fgets(linha, sizeof(linha), arquivo)) {
        // CORRIGIDO: Máscara sscanf com espaços para robustez
        int result = sscanf(linha, "%d ,%49[^,], %49[^,], %19[^,], %d ,%19[^,], %49[^,], %d ,%d ,%d ,%99[^\n]",
               &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma);

        if (result != 11) {
             // Se houver falha de leitura, escreve a linha original e pula a alteração.
            fprintf(temp, "%s", linha); 
            continue; 
        }

        // REQUISITO ATENDIDO: APENAS ALUNOS PODEM TER NOTAS ATUALIZADAS.
        if (id == idBusca && strcmp(nivel, "aluno") == 0) {
            printf("Aluno encontrado: %s (%s)\n", nome, turma);
            printf("Nova NP1: "); scanf("%d", &np1);
            printf("Nova NP2: "); scanf("%d", &np2);
            printf("Novo PIM: "); scanf("%d", &pim);
            limpar_buffer_entrada();
            encontrado = 1;
        }

        // Garantir que a turma não tenha o '\n' antes de reescrever
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
    
    // Correção de segurança
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

// FUNÇÃO CORRIGIDA: Mostra todos os usuários
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
    
    // Pula o cabeçalho (apenas lê)
    if (fgets(linha, sizeof(linha), arquivo)) {
        // Imprime o cabeçalho formatado para alinhamento
        printf("------------------------------------------------------------------------------------------------------------------------------------------------\n");
        printf("| ID | NOME               | EMAIL              | NIVEL       | IDADE | CURSO              | NP1 | NP2 | PIM | TURMA                           |\n");
        printf("------------------------------------------------------------------------------------------------------------------------------------------------\n");
    }
    
    // Imprime as linhas de dados analisando com sscanf
    while (fgets(linha, sizeof(linha), arquivo)) {
        // CORRIGIDO: Máscara sscanf com espaços para robustez
        int result = sscanf(linha, "%d ,%49[^,], %49[^,], %19[^,], %d ,%19[^,], %49[^,], %d ,%d ,%d ,%99[^\n]",
                            &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma);

        // Se o sscanf foi bem-sucedido (esperamos 11 campos), formata a saída
        if (result == 11) {
            // Remove o '\n' que pode ter sido lido no final da turma, caso a linha não termine exatamente com um caractere
            turma[strcspn(turma, "\n")] = '\0';
            printf("| %-2d | %-18s | %-18s | %-11s | %-5d | %-18s | %-3d | %-3d | %-3d | %-30s|\n",
                   id, nome, email, nivel, idade, curso, np1, np2, pim, turma);
        } else {
            // Caso a linha não esteja no formato esperado, imprime a linha bruta com um aviso.
            linha[strcspn(linha, "\n")] = '\0'; 
            printf("| ERRO NA LEITURA DA LINHA (result=%d): %s\n", result, linha);
        }
    }
    
    printf("------------------------------------------------------------------------------------------------------------------------------------------------\n");
    fclose(arquivo);
}

// NOVO: Gerenciar usuários
void gerenciarUsuarios() {
    int op;
    do {
        printf("\n=== GERENCIAR USUÁRIOS ===\n");
        printf("1 - Mostrar todos os usuários\n");
        printf("2 - Cadastrar novo usuário\n");
        printf("0 - Voltar ao Menu Admin\n");
        printf("Opção: ");
        scanf("%d", &op);
        limpar_buffer_entrada();

        switch (op) {
            case 1: mostrarTodosUsuarios(); break;
            case 2: cadastrarUsuarios(); break;
            case 0: break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

// NOVO: Gerenciar turmas
void gerenciarTurmas() {
    printf("\n=== TURMAS DO SISTEMA ===\n");
    for (int i = 0; i < TAM_TURMA; i++) {
        printf("%d - %s\n", i + 1, TURMAS[i]);
    }
    printf("===========================\n");
    
    printf("Pressione ENTER para voltar ao menu admin...");
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
        scanf("%d", &op);
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
        scanf("%d", &op);
        limpar_buffer_entrada();

        switch (op) {
            case 1: atualizarNotas(); break;
            case 2: enviarExercicioPDF(); break;
            case 0: printf("Saindo do menu professor...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (op != 0);
}

// Menu Aluno
void menuAluno(const char *nome, int np1, int np2, int pim, const char *turma) {
    int op;
    do {
        printf("\n=== MENU ALUNO ===\n");
        printf("1 - Ver notas e média\n");
        printf("2 - Acessar vídeo-aula da turma\n");
        printf("0 - Sair\n");
        printf("Opção: ");
        scanf("%d", &op);
        limpar_buffer_entrada();

        switch (op) {
            case 1: {
                float media = (np1 * 4 + np2 * 4 + pim * 2) / 10.0;
                printf("\nNotas → NP1: %d | NP2: %d | PIM: %d | Média Final: %.2f\n", np1, np2, pim, media);
                break;
            }
            case 2:
                printf("\n🎥 Acessando vídeo-aula da turma: %s\n", turma);
                // A URL pode não funcionar perfeitamente com espaços. É apenas um placeholder.
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

    fgets(linha, sizeof(linha), arquivo); // pula cabeçalho
    while (fgets(linha, sizeof(linha), arquivo)) {
        // CORRIGIDO: Máscara sscanf com espaços para robustez
        sscanf(linha, "%d ,%49[^,], %49[^,], %19[^,], %d ,%19[^,], %49[^,], %d ,%d ,%d ,%99[^\n]",
               &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim, turma);

        if (strcmp(email_login, email) == 0 && strcmp(senha_login, senha) == 0) {
            encontrado = 1;
            // Remove o '\n' que pode ter sido lido no final da turma
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

    if (strcmp(nivel, "professor") == 0)
        menuProfessor();
    else if (strcmp(nivel, "admin") == 0 || strcmp(nivel, "coordenador") == 0) // Coordenador usa menu admin
        menuAdmin();
    else if (strcmp(nivel, "aluno") == 0)
        menuAluno(nome, np1, np2, pim, turma);
    else
        printf("Nível sem menu específico.\n");
}

// NOVO: Inicialização do arquivo CSV
void inicializarArquivoCSV() {
    FILE *arquivo = fopen(NOME_ARQUIVO, "a+");
    if (!arquivo) {
        // Se a pasta não existir, pode falhar. Assumimos que C:\estudos existe.
        printf("ERRO FATAL: Não foi possível criar ou abrir o arquivo de usuários.\n");
        exit(1);
    }
    
    // Move o ponteiro para o início para verificar o tamanho
    fseek(arquivo, 0, SEEK_END);
    if (ftell(arquivo) == 0) {
        // Arquivo está vazio, escreve o cabeçalho
        fprintf(arquivo, "ID,NOME,EMAIL,SENHA,IDADE,NIVEL,CURSO,NP1,NP2,PIM,TURMA\n");
    }
    fclose(arquivo);
}

// Principal
int main() {
    inicializarArquivoCSV(); // Garante que o arquivo e o cabeçalho existam
    
    int opc;
    do {
        printf("\n===== SISTEMA ACADÊMICO =====\n");
        printf("1 - Cadastrar usuário\n");
        printf("2 - Login\n");
        printf("0 - Sair\n");
        printf("Opção: ");
        scanf("%d", &opc);
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