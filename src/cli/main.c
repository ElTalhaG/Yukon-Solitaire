#include <stdio.h>

#include "app.h"
#include "deck_io.h"
#include "game.h"

int main(void)
{
    /*
     * This temporary CLI entry point exists so CLion can configure, build,
     * and run the project immediately. The real terminal command loop will
     * replace this once the parser and renderer phases are implemented.
     * Meaning not yet, but soon will we implement it.
     */
    return run_cli_application();
}
