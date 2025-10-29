#include <stdio.h>
#include <string.h>
#include "Sistema_Academico.h" // <-- Inclui o .h 

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
            // --- CORREÇÃO APLICADA AQUI ---
            // Passamos "public" para a função.
            // A lógica em cadastrarUsuarios() usará isso para impedir
            // que um admin seja criado por este menu se outro já existir.
            case 1: cadastrarUsuarios("public"); break; 
            // --- FIM DA CORREÇÃO ---
            
            case 2: loginUsuario(); break;
            case 0: printf("Encerrando...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (opc != 0);
    return 0;
}