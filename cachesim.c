# include <stdlib.h>
# include <stdio.h>
# include <math.h>

typedef struct cache_line_t {
    int* use_order;
    int* tag;       // 1 or more tags
} Cache_Line;

typedef struct cache_t {
    int sets;
    int dir;
    int line_size;
    Cache_Line** lines;
} Cache;

void freecache(Cache* cache)
{
    for (int i = 0; i < cache->sets; i++)
    {
        free(cache->lines[i]->use_order);
        free(cache->lines[i]->tag);
        free(cache->lines[i]);
    }
    free(cache);
}

int get_index(int val, int* val_list, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (val_list[i] == val)
            return i;
    }
    return -1;
}

int get_least_recently_used(int* order_list, int len)
{
    int max = order_list[0];
    int lru = 0;
    for (int i = 1; i < len; i++)
    {
        if ( order_list[i] > max ) {
            max = order_list[i];
            lru = i;
        }
    }
    return lru;
}

void update_use(int* order_list, int new_least, int len)
{
    if (order_list[new_least] != 0) {
        for (int i = 0; i < len; i++)
        {
            order_list[i] = order_list[i] + 1;
        }
        order_list[new_least] = 0;
    }
}

Cache* create_cache(int line_size, int directories, int sets)
{
    Cache_Line** entries = malloc( sets*sizeof(Cache_Line*) );
    for( int i = 0; i < sets; i++ )
    {
        Cache_Line* line = malloc( sizeof(Cache_Line) );
        line->tag = malloc( directories*sizeof(int) );
        int* tags = malloc( directories*sizeof(int) );
        for (int j = 0; j < directories; j++)
        {
            tags[j] = j;
            line->tag[j] = -1;
        }
        line->use_order = tags;
        entries[i] = line;
    }
    
    Cache* cache = malloc( sizeof(Cache) );
    cache->sets = sets;
    cache->dir = directories;
    cache->line_size = line_size;
    cache->lines = entries;
 
    return cache;
}

int get_cache_hits(Cache* cache, int* access_list, int list_len)
{
    int hits = 0;
    int set_width = ceil(log((double)cache->sets));
    int mask = 0;
    for (int t = set_width; t > 0; t--)
        mask = (mask << 1) + 1;
    
    for (int i = 0; i < list_len; i++)
    {
        unsigned short address = access_list[i];
        int offset = address & 0x000F;

        int set = (address >> 4) & mask;
        int tag = address >> 4 + set_width;
        
        Cache_Line* this_line = cache->lines[set];
        printf("addr = 0x%4x : set = %d :   ", address, set);
        
        int k = get_index(tag, this_line->tag, cache->dir);
        if ( k != -1 ) {
            hits++; // I guess ya never miss huh
            printf("hit\n");
            if (cache->dir > 1)
                update_use(this_line->use_order, k, cache->dir);
        }
        else {
            printf("miss\n");
            int least = get_least_recently_used(this_line->use_order, cache->dir);
            update_use(this_line->use_order, least, cache->dir);
            this_line->tag[least] = tag;
        }
    }
    return hits;
}

int main(int argc, char** argv)
{
    int directories;
    int sets;
    int line_size;

    if (argc == 4) {
      directories = atoi(argv[1]);
      sets = atoi(argv[2]);
      line_size = atoi(argv[3]);
    }
    else {
      printf("Using defaults. Dir = 1, Sets = 8\n\n");
      directories = 1;
      sets = 8;
      line_size = 16;
    }
    int addresses[] 
          = { 0x0000, 0x0004, 0x000c, 0x2200, 0x00d0, 0x00e0, 0x1130, 0x0028,
              0x113c, 0x2204, 0x0010, 0x0020, 0x0004, 0x0040, 0x2208, 0x0008,
              0x00a0, 0x0004, 0x1104, 0x0028, 0x000c, 0x0084, 0x000c, 0x3390,
              0x00b0, 0x1100, 0x0028, 0x0064, 0x0070, 0x00d0, 0x0008, 0x3394 };         
    int list_len = 32;

    Cache* cache = create_cache(line_size, directories, sets);
    int hits = get_cache_hits(cache, addresses, list_len);
    int misses = list_len - hits;

    printf("\nHits: %d\nMisses: %d\nTotal: %d\n", hits, misses, list_len);

    freecache(cache);
}
