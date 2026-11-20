int token_counter = 1000;          /* single definition, external linkage, static storage */
int next_token_ext(void) { return ++token_counter; }
