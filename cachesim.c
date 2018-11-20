# include <stdlib.h>
# include <math.h>

struct cache_t {
    int sets;
    int dir;
    int line_size;
    struct Cache_Line* lines;
} Cache;

struct cache_line_t {
    int* use_order;
    int* tag;       // 1 or more tags
} Cache_Line;

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
    for (int i = 0; i < len; i++)
    {
        if ( order_list[i] == len-1 )
            return i;
    }
    return 0;
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
    CacheLine entries[sets];
    for( int i = 0; i < sets; i++ )
    {
        CacheLine line;
        line.tag = null;

        int tags[directories];
        for (int j = 0; j < directories; j++)
        {
            tags[j] = j;
        }

        line.use_order = tags;
        entries[i] = line;
    }
    Cache cache;
    cache.sets = sets;
    cache.dir = directories;
    cache.line_size = line_size;
    cache.lines = entries;

    return &cache;
}

int get_cache_hits(Cache* cache, int* access_list, int list_len)
{
    int hits = 0;
    int set_width = log(cache->sets);
    for (int i = 0; i < list_len; i++)
    {
        unsigned short address = access_list[i];
        int offset = address & 0x000F;
        int set = (address << (12-set_width)) >> (12-set_width*2);
        int tag = address >> 4 + set_width;

        CacheLine this_line = cache->lines[i];
        int k = get_index(tag, this_line.tag, cache->dir);

        if ( k != -1 ) {
            hit++; // I guess ya never miss huh
            if (cache->dir > 1)
                update_use(this_line.use_order, k);
        }
        else {
            int least = get_least_recently_used(this_line.use_order, cache->dir);
            update_use(this_line.use_order, least, cache->dir);
            this_line.tag[least] = tag;
        }
    }
    return hits;
}

int main(int argc, char** argv)
{
    int directories;
    int sets;
    int line_size;

    // user input

    int* addresses;
    int list_len;
    // parse addresses

    Cache* cache = create_cache(line_size, directories, sets);

    int hits = get_cache_hits(cache, addresses, list_len);
    int misses = list_len - hits;

    printf("Hits: %d\nMisses: %d\nTotal: %d", hits, misses, list_len);
}