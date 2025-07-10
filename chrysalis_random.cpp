#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <sys/time.h>
using namespace std;

const int SZ = 200;
const int THRESHOLD = 50;
const int MAX_POPULATION = SZ * SZ;
const int OUTPUT_MIN = 500;
int keai[MAX_POPULATION + 1];

struct Point {
    int x, y;
    Point() {}
    Point(int x_, int y_) : x(x_), y(y_) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    bool operator<(const Point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
};

typedef vector<Point> Polyomino;

vector<int> init_rules() {
    vector<int> rules(512, 0);
    for (int c = 0; c < 512; ++c) {
        bool cen = (c >> 4) & 1;
        int cnt = ((c >> 8) & 1) + ((c >> 7) & 1) + 
                 ((c >> 6) & 1) + ((c >> 5) & 1) +
                 ((c >> 3) & 1) + ((c >> 2) & 1) + 
                 ((c >> 1) & 1) + (c & 1);
        rules[c] = cen ? (cnt == 2 || cnt == 3) : (cnt == 3);
    }
    rules[0x101] = rules[0x044] = 1;
    rules[0x191] = rules[0x05C] = rules[0x113] = rules[0x074] = 
    rules[0x0D4] = rules[0x131] = rules[0x056] = rules[0x119] = 0;
    return rules;
}

int next_gen(const vector<vector<int> >& cur,
             vector<vector<int> >& nxt,
             const vector<int>& rules) {
    int cnt = 0;
    for (int i = 0; i < SZ; ++i) {
        for (int j = 0; j < SZ; ++j) {
            int code = 0;
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    int x = (i + dx + SZ) % SZ;
                    int y = (j + dy + SZ) % SZ;
                    code = (code << 1) | cur[x][y];
                }
            }
            nxt[i][j] = rules[code];
            cnt += nxt[i][j];
        }
    }
    return cnt;
}

void place_polyomino(vector<vector<int> >& grid, const Polyomino& poly) {
    int minX = SZ, maxX = 0, minY = SZ, maxY = 0;
    for (Polyomino::const_iterator it = poly.begin(); it != poly.end(); ++it) {
        minX = min(minX, it->x);
        maxX = max(maxX, it->x);
        minY = min(minY, it->y);
        maxY = max(maxY, it->y);
    }
    
    int offsetX = (SZ - (maxX - minX + 1)) / 2;
    int offsetY = (SZ - (maxY - minY + 1)) / 2;
    
    for (Polyomino::const_iterator it = poly.begin(); it != poly.end(); ++it) {
        int x = it->x - minX + offsetX;
        int y = it->y - minY + offsetY;
        if (x >= 0 && x < SZ && y >= 0 && y < SZ) {
            grid[x][y] = 1;
        }
    }
}

void print_grid(const vector<vector<int> >& grid, int gen) {
    int min_x = SZ, max_x = -1;
    int min_y = SZ, max_y = -1;
    
    for (int i = 0; i < SZ; ++i) {
        for (int j = 0; j < SZ; ++j) {
            if (grid[i][j]) {
                min_x = min(min_x, i);
                max_x = max(max_x, i);
                min_y = min(min_y, j);
                max_y = max(max_y, j);
            }
        }
    }

    min_x = max(0, min_x-1);
    max_x = min(SZ-1, max_x+1);
    min_y = max(0, min_y-1);
    max_y = min(SZ-1, max_y+1);

    cout << "(" 
         << (max_x-min_x-1) << "x" << (max_y-min_y-1) << "):\n";
         
    for (int i = min_x; i <= max_x; ++i) {
        for (int j = min_y; j <= max_y; ++j) {
            cout << (grid[i][j] ? 'o' : '.');
        }
        cout << '\n';
    }
}

int calculate_lifespan(const Polyomino& poly, const vector<int>& rules) {
    vector<vector<int> > grid(SZ, vector<int>(SZ, 0)), grid2(SZ, vector<int>(SZ, 0));
    place_polyomino(grid, poly);
    grid2 = grid;
    
    memset(keai, 0, sizeof(keai));
    int estimated = 0, gen = 0;
    
    do {
        vector<vector<int> > next(SZ, vector<int>(SZ, 0));
        int pop = next_gen(grid, next, rules);
        
        if (keai[pop] == 0 || gen - keai[pop] >= THRESHOLD) estimated = gen;
        keai[pop] = ++gen;
        
        grid.swap(next);
    } while (gen - estimated <= THRESHOLD && gen < 100000);

    if (estimated >= OUTPUT_MIN) print_grid(grid2, gen);
    return gen - estimated > THRESHOLD ? estimated : -1;
}

unsigned int get_seed() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned int)(tv.tv_sec * 1000000 + tv.tv_usec) ^ getpid() ^ clock();
}

Polyomino generate_random_10x10() {
    Polyomino poly;
    unsigned int seed = get_seed();
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            seed = (seed >> 1) ^ (-(seed & 1) & 0xD0000001);
            if ((seed >> 16) & 1) {
                poly.push_back(Point(i, j));
            }
        }
        seed ^= clock() + get_seed();
    }
    return poly;
}

int main() {
    freopen("out4.txt", "w", stdout);
    vector<int> rules = init_rules();

    for (int i = 0; ; ++i) {
        Polyomino random_poly = generate_random_10x10();
        int lifespan = calculate_lifespan(random_poly, rules);
        cout << "Sample " << (i+1) << " lifespan: " << lifespan << "\n\n";
    }

    return 0;
}
