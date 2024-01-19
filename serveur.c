#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define MAX_MESSAGE_LENGTH 1024

// Déclaration de variables globales volatiles (modifiables par les gestionnaires de signaux)
volatile int bit_count = 0;                   // Nombre de bits actuellement reçus pour le caractère en cours
volatile char message[MAX_MESSAGE_LENGTH];    // Buffer pour stocker le message reçu
volatile int current_bit = 0;                 // Dernier bit reçu
volatile int current_char_index = 0;          // Index du caractère en cours dans le message

// Fonction pour réinitialiser le message après sa réception
void reset_message() {
    memset((void *)message, 0, current_char_index + 1);
    current_char_index = 0;
}

// Fonction pour journaliser le message reçu avec un horodatage
void log_message(const char *received_message) {
    time_t now;
    char timestamp[32];

    time(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    printf("[%s] Message reçu: %s\n", timestamp, received_message);
}

// Gestionnaire de signaux pour SIGUSR1 et SIGUSR2
void handle_signal(int signum) {
    current_bit = (signum == SIGUSR2) ? 1 : 0; // Définir le bit actuel en fonction du signal reçu
    message[current_char_index] = (message[current_char_index] << 1) | current_bit; // Construire le caractère bit par bit
    bit_count++;

    if (bit_count == 8) { // Si 8 bits ont été reçus (1 octet), passer au caractère suivant
        bit_count = 0;
        current_char_index++;

        if (message[current_char_index - 1] == '\0') { // Si le caractère nul est atteint, fin du message
            log_message(message); // Journaliser le message
            reset_message(); // Réinitialiser pour le prochain message
        }
    }
}

// Configuration des gestionnaires de signaux
void setup_signal_handling() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_RESTART; // Redémarrer les appels système si interrompus
    sa.sa_handler = handle_signal; // Assigner le gestionnaire de signaux

    // Configuration de sigaction pour SIGUSR1 et SIGUSR2
    if (sigaction(SIGUSR1, &sa, NULL) == -1 || sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("Erreur lors de la configuration de sigaction");
        exit(EXIT_FAILURE); // Quitter en cas d'erreur
    }
}

// Fonction principale
int main() {
    printf("Serveur PID: %d\n", getpid()); // Afficher l'ID du processus
    setup_signal_handling(); // Configurer les gestionnaires de signaux

    while (1) {
        pause(); // Attendre indéfiniment les signaux
    }

    return EXIT_SUCCESS; // Fin du programme (inatteignable dans ce cas)
}
