
int allocate_cont(int num_pages, int page_size, KGSLEntry *ret_entries);
void print_entries(KGSLEntry entries[], int start_idx, int num_to_print);
void createTexture2DRGBA(unsigned int textureId, uint32_t *data, GLuint width, GLuint height);