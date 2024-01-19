#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define DELAY 500 // Définir délai en microsecondes envoie-signal

// Fonction pour envoyer un signal au processus serveur
void send_signal(pid_t server_pid, int signal) {
    if (kill(server_pid, signal) == -1) {
        perror("Erreur lors de l'envoi du signal"); // Si kill échoue=erreur
        exit(EXIT_FAILURE); // si erreur=quitte le programme 
    }
    usleep(DELAY); // Pause entre les envois de signaux pour garantir la réception
}

// Fonction pour envoyer un caractère au serveur sous forme de séquence de signaux
void send_char_as_signals(char c, pid_t server_pid) {
    for (int i = 7; i >= 0; --i) { // Boucle pour envoyer chaque bit du caractère
        int bit = (c >> i) & 1; // Isoler le bit actuel (commence par le bit de poids le plus élevé)
        send_signal(server_pid, bit ? SIGUSR2 : SIGUSR1); // Envoyer SIGUSR2 pour 1, SIGUSR1 pour 0
    }
}

// Fonction pour valider les arguments passés au programme
int validate_arguments(int argc, char *argv[]) {
    if (argc != 3 || atoi(argv[1]) <= 0) { // Vérifie le nombre d'arguments et la validité du PID
        fprintf(stderr, "Usage: %s <Server PID> <Message>\n", argv[0]); // Message d'erreur si invalide
        return 0; // Retourne 0 pour signaler une invalidité
    }
    return 1; // Retourne 1 si les arguments sont valides
}

int main(int argc, char *argv[]) {
    if (!validate_arguments(argc, argv)) { // Valide les arguments avant de continuer
        return 1; // Termine le programme si les arguments ne sont pas valides
    }

    pid_t server_pid = (pid_t)atoi(argv[1]); // Convertit le PID de chaîne en nombre
    char *message = argv[2]; // Pointe vers le message à envoyer

    // Boucle pour envoyer le message, caractère par caractère
    while (*message) {
        send_char_as_signals(*message++, server_pid); // Envoie chaque caractère
    }

    // Envoie un caractère nul pour indiquer la fin du message
    send_char_as_signals('\0', server_pid);

    printf("Message envoyé au serveur avec le PID %d\n", server_pid); // Confirme l'envoi du message
    return 0; // Termine le programme avec succès
}
