#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <climits>
using namespace std;

const int SZ = 200;
const int THRESHOLD = 50;
const int MAX_POPULATION = SZ * SZ;
const int OUTPUT_MIN = 300;
int keai[MAX_POPULATION + 1] = {0};

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    bool operator<(const Point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
};

typedef vector<Point> Polyomino;

const int dx[] = {-1, 0, 1, 0};
const int dy[] = {0, 1, 0, -1};

struct PointCompare {
    bool operator()(const Point& a, const Point& b) const {
        return a < b;
    }
};

class PolyominoGenerator {
    set<Polyomino> unique_poly;
    int current_n;

    void normalize(Polyomino& p) {
        if (p.empty()) return;
        
        int minX = p[0].x, minY = p[0].y;
        for (size_t i = 1; i < p.size(); ++i) {
            minX = min(minX, p[i].x);
            minY = min(minY, p[i].y);
        }
        
        for (size_t i = 0; i < p.size(); ++i) {
            p[i].x -= minX;
            p[i].y -= minY;
        }
        
        sort(p.begin(), p.end());
        p.erase(unique(p.begin(), p.end()), p.end());
    }

    Polyomino rotate(const Polyomino& p) {
        Polyomino r;
        for (size_t i = 0; i < p.size(); ++i) {
            r.push_back((Point){p[i].y, -p[i].x});
        }
        normalize(r);
        return r;
    }

    Polyomino reflect(const Polyomino& p) {
        Polyomino s;
        for (size_t i = 0; i < p.size(); ++i) {
            s.push_back((Point){-p[i].x, p[i].y});
        }
        normalize(s);
        return s;
    }

    void generateSymmetries(const Polyomino& p, set<Polyomino>& symmetries) {
        Polyomino current = p;
        for (int i = 0; i < 4; ++i) {
            symmetries.insert(current);
            symmetries.insert(reflect(current));
            current = rotate(current);
        }
    }

    Polyomino getCanonicalForm(const Polyomino& p) {
        set<Polyomino> symmetries;
        generateSymmetries(p, symmetries);
        return *symmetries.begin();
    }

    void dfs(Polyomino current, set<Point, PointCompare> visited, Point next) {
        if (find(current.begin(), current.end(), next) != current.end()) 
            return;
        
        visited.insert(next);
        current.push_back(next);
        normalize(current);

        if (current.size() != current_n) {
            Point last = current.back();
            for (int d = 0; d < 4; ++d) {
                Point neighbor = {last.x + dx[d], last.y + dy[d]};
                if (visited.find(neighbor) == visited.end()) {
                    dfs(current, visited, neighbor);
                }
            }
            return;
        }

        Polyomino canonical = getCanonicalForm(current);
        if (unique_poly.find(canonical) == unique_poly.end()) {
            unique_poly.insert(canonical);
        }
    }

public:
    vector<Polyomino> generate(int n) {
        unique_poly.clear();
        current_n = n;
        set<Point, PointCompare> initVisited;
        dfs(Polyomino(), initVisited, (Point){0,0});
        return vector<Polyomino>(unique_poly.begin(), unique_poly.end());
    }
};

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
	rules[0x191] = rules[0x05C] = rules[0x113] = rules[0x074] = rules[0x0D4] = rules[0x131] = rules[0x056] = rules[0x119] = 0; // LeapLife
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
    place_polyomino(grid, poly); place_polyomino(grid2, poly); 
    
    fill_n(keai, MAX_POPULATION + 1, 0);
    int estimated = 0, gen = 0;
    
    do {
        //print_grid(grid, gen);
        vector<vector<int> > next(SZ, vector<int>(SZ, 0));
        int pop = next_gen(grid, next, rules);
        
        if (!keai[pop] || gen - keai[pop] >= THRESHOLD) estimated = gen;
        keai[pop] = ++gen;
        
        grid.swap(next);
    } while (gen - estimated <= THRESHOLD);

    if (estimated >= OUTPUT_MIN) print_grid(grid2, gen);
    return gen - estimated > THRESHOLD ? estimated : -1;
}

int main() {
	freopen("out.txt", "w", stdout);
    PolyominoGenerator pg;
    vector<int> rules = init_rules();
    
    for (int n = 1; n <= 12; ++n) {
        vector<Polyomino> polys = pg.generate(n);
        cout << "Found " << polys.size() << " free polyominoes of size " << n << endl;
        
        for (vector<Polyomino>::const_iterator it = polys.begin(); it != polys.end(); ++it) {
            int lifespan = calculate_lifespan(*it, rules);
            cout << "Estimated lifespan: " << lifespan << endl;
        }
    }
    
    return 0;
}
