#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define MAX_MESSAGE_LENGTH 1024

// Variables pr gérer l'état du message reçu
volatile int bit_count = 0;                   // Compteur de bits actuellement reçus pour le caractère en cours
volatile char message[MAX_MESSAGE_LENGTH];    // Buffer pour stocker le message reçu
volatile int current_bit = 0;                 // Dernier bit reçu (0 ou 1)
volatile int current_char_index = 0;          // Index du caractère actuel dans le message
FILE *log_file = NULL;                        // Pointeur vers le fichier de journalisation

// Fonction pour ajouter un horodatage et écrire le message dans le fichier log
void log_message(const char *received_message) {
    // Obtenir l'horodatage actuel
    time_t now = time(NULL);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    // Écrire le message avec l'horodatage dans le fichier log
    fprintf(log_file, "[%s] %s\n", timestamp, received_message);
    fflush(log_file); // S'assurer que le contenu est immédiatement écrit sur le disque
}

// Fonction pour réinitialiser le buffer du message une fois le message complet reçu
void reset_message() {
    memset((void *)message, 0, current_char_index + 1);
    current_char_index = 0;
}

// Gestionnaire de signaux pour SIGUSR1 et SIGUSR2
void handle_signal(int signum) {
    // Définir le bit actuel en fonction du signal reçu et construire le caractère
    current_bit = (signum == SIGUSR2) ? 1 : 0;
    message[current_char_index] = (message[current_char_index] << 1) | current_bit;
    bit_count++;

    // Si 8 bits ont été reçus (formant un octet), passer au caractère suivant
    if (bit_count == 8) {
        bit_count = 0;
        current_char_index++;

        // Si le caractère nul est atteint, cela indique la fin du message
        if (message[current_char_index - 1] == '\0') {
            log_message(message); // Journaliser le message
            reset_message();      // Réinitialiser le buffer pour le prochain message
        }
    }
}

// Configuration des gestionnaires de signaux pour l'écoute des signaux
void setup_signal_handling() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal; // Définir la fonction de rappel pour les signaux
    sigaction(SIGUSR1, &sa, NULL); // Configurer pour le signal SIGUSR1
    sigaction(SIGUSR2, &sa, NULL); // Configurer pour le signal SIGUSR2
}

int main() {
    // Ouvrir ou créer le fichier de log pour écrire les messages reçus
    log_file = fopen("conversations.log", "a");
    if (log_file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de log");
        exit(EXIT_FAILURE); // Quitter si le fichier ne peut pas être ouvert
    }

    setup_signal_handling(); // Configurer les gestionnaires de signaux

    printf("Serveur prêt. PID: %d\n", getpid()); // Afficher le PID du serveur pour le client

    // Boucle infinie pour attendre les signaux de manière continue
    while (1) {
        pause(); // Met le processus en pause jusqu'à ce qu'un signal soit reçu
    }

    // Fermeture du fichier de log lors de la fin du programme (ce code n'est pas atteint dans cet exemple)
    fclose(log_file);
    
    return 0; // Fin du programme (en pratique, ce code n'est pas atteint)
}
