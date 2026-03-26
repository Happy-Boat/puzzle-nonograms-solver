#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;

struct Nonogram {
    int rows, cols;
    vector<vector<int>> row_clues;
    vector<vector<int>> col_clues;
    // 'x' = filled, 'o' = empty, '?' = unknown
    vector<vector<char>> board;
};

bool loadPuzzle(const string& filename, Nonogram& p) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Unable to open file: " << filename << endl;
        return false;
    }

    string line, tag;
    if (!(file >> p.rows >> p.cols)) return false;
    p.board.assign(p.rows, vector<char>(p.cols, '?'));

    while (file >> tag) {
        if (tag == "ROWS") {
            getline(file, line); // consume rest of line
            for (int i = 0; i < p.rows; i++) {
                getline(file, line);
                stringstream ss(line);
                vector<int> clue;
                int n;
                while (ss >> n) clue.push_back(n);
                if (clue.empty()) clue.push_back(0);
                p.row_clues.push_back(clue);
            }
        } else if (tag == "COLS") {
            getline(file, line);
            for (int i = 0; i < p.cols; i++) {
                getline(file, line);
                stringstream ss(line);
                vector<int> clue;
                int n;
                while (ss >> n) clue.push_back(n);
                if (clue.empty()) clue.push_back(0);
                p.col_clues.push_back(clue);
            }
        }
    }
    return true;
}

void printBoard(const Nonogram& p) {
    cout << "\n===== Nonogram Solution (" 
         << p.rows << "x" << p.cols << ") =====\n" << endl;
    for (int i = 0; i < p.rows; i++) {
        cout << "  ";
        for (int j = 0; j < p.cols; j++) {
            if (p.board[i][j] == 'x')      cout << "x ";
            else if (p.board[i][j] == 'o') cout << "o ";
            else                           cout << "? ";
        }
        cout << endl;
    }
    cout << endl;
}

// ==================== Core: Row/Column Solver ====================
// Given the current state of a line and the clues, uses forward-backward DP
// to determine which cells must be filled or empty. Updates the line.
// Returns false if no valid solution exists.
bool solveLine(vector<char>& line, const vector<int>& clues) {
    int n = (int)line.size();
    int k = (int)clues.size();

    // Special case: no blocks -> all cells must be empty
    if (k == 0 || (k == 1 && clues[0] == 0)) {
        for (int i = 0; i < n; i++) {
            if (line[i] == 'x') return false; // conflict: already filled but must be empty
            line[i] = 'o';
        }
        return true;
    }

    // Prefix sum of empty cells for O(1) range queries
    // canFill(a,b) == true if interval [a,b) contains no forced empty cells
    vector<int> pre(n + 1, 0);
    for (int i = 0; i < n; i++)
        pre[i + 1] = pre[i] + (line[i] == 'o');
    auto canFill = [&](int a, int b) -> bool {
        return pre[b] - pre[a] == 0;
    };

    // ---------- Forward DP ----------
    // fwd[j][i] = whether we can reach state (j, i) from the start (0,0)
    //   State: placed first j blocks, next cell to consider is i
    vector<vector<bool>> fwd(k + 1, vector<bool>(n + 1, false));
    fwd[0][0] = true;

    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= k; j++) {
            if (!fwd[j][i]) continue;

            // Transition 1: skip cell i (empty), allowed only if i is not forced filled
            if (i < n && line[i] != 'x')
                fwd[j][i + 1] = true;

            // Transition 2: place block j in [i, i+clues[j])
            if (j < k) {
                int e = i + clues[j]; // end position (exclusive)
                if (e <= n && canFill(i, e)) {
                    if (e == n)
                        fwd[j + 1][n] = true;          // block ends exactly at line end
                    else if (line[e] != 'x')
                        fwd[j + 1][e + 1] = true;      // one empty cell after the block
                }
            }
        }
    }

    // No valid placement from start to end
    if (!fwd[k][n]) return false;

    // ---------- Backward DP ----------
    // bwd[j][i] = whether we can reach the goal (k,n) from state (j,i)
    vector<vector<bool>> bwd(k + 1, vector<bool>(n + 1, false));
    bwd[k][n] = true;

    for (int i = n; i >= 0; i--) {
        for (int j = k; j >= 0; j--) {
            // Reverse transition 1
            if (i < n && line[i] != 'x' && bwd[j][i + 1])
                bwd[j][i] = true;

            // Reverse transition 2
            if (j < k) {
                int e = i + clues[j];
                if (e <= n && canFill(i, e)) {
                    if (e == n && bwd[j + 1][n])
                        bwd[j][i] = true;
                    else if (e < n && line[e] != 'x' && bwd[j + 1][e + 1])
                        bwd[j][i] = true;
                }
            }
        }
    }

    // ---------- Determine each cell ----------
    // For every transition, if fwd[source] && bwd[target] are true,
    // that transition participates in at least one valid solution.
    vector<int> fill_diff(n + 1, 0);   // difference array to count "can be filled"
    vector<bool> can_empty(n, false);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= k; j++) {
            if (!fwd[j][i]) continue;

            // Transition 1 is valid -> cell i can be empty
            if (line[i] != 'x' && bwd[j][i + 1])
                can_empty[i] = true;

            // Transition 2 is valid -> cells [i, e) can be filled, cell e can be empty
            if (j < k) {
                int e = i + clues[j];
                if (e <= n && canFill(i, e)) {
                    bool ok = false;
                    if (e == n)
                        ok = bwd[j + 1][n];
                    else if (line[e] != 'x')
                        ok = bwd[j + 1][e + 1];
                    if (ok) {
                        fill_diff[i]++;    // start of fill range
                        fill_diff[e]--;    // end of fill range
                        if (e < n) can_empty[e] = true;
                    }
                }
            }
        }
    }

    // Reconstruct can_filled from the difference array
    vector<bool> can_filled(n, false);
    for (int i = 0, s = 0; i < n; i++) {
        s += fill_diff[i];
        can_filled[i] = (s > 0);
    }

    // Update the line
    for (int i = 0; i < n; i++) {
        if (!can_filled[i] && !can_empty[i])
            return false; // contradiction: cell cannot be either filled or empty
        if (line[i] == '?') {
            if (can_filled[i] && !can_empty[i])
                line[i] = 'x';  // must be filled
            else if (!can_filled[i] && can_empty[i])
                line[i] = 'o';  // must be empty
        }
    }
    return true;
}

// ==================== Constraint Propagation ====================
// Repeatedly apply solveLine to all rows and columns until no more changes.
// Returns false if a contradiction is found.
bool propagate(Nonogram& p) {
    bool changed = true;
    while (changed) {
        changed = false;

        // Process all rows
        for (int i = 0; i < p.rows; i++) {
            vector<char> line = p.board[i];
            if (!solveLine(line, p.row_clues[i])) return false;
            if (line != p.board[i]) {
                p.board[i] = line;
                changed = true;
            }
        }

        // Process all columns
        for (int j = 0; j < p.cols; j++) {
            vector<char> line(p.rows);
            for (int i = 0; i < p.rows; i++) line[i] = p.board[i][j];
            if (!solveLine(line, p.col_clues[j])) return false;
            for (int i = 0; i < p.rows; i++) {
                if (p.board[i][j] != line[i]) {
                    p.board[i][j] = line[i];
                    changed = true;
                }
            }
        }
    }
    return true;
}

// ==================== Helper Functions ====================
bool isSolved(const Nonogram& p) {
    for (auto& row : p.board)
        for (char c : row)
            if (c == '?') return false;
    return true;
}

pair<int, int> findUnknown(const Nonogram& p) {
    for (int i = 0; i < p.rows; i++)
        for (int j = 0; j < p.cols; j++)
            if (p.board[i][j] == '?') return {i, j};
    return {-1, -1};
}

// ==================== Main Solver: Propagation + Backtracking ====================
bool solve(Nonogram& p) {
    // Step 1: propagate constraints as much as possible
    if (!propagate(p)) return false;  // contradiction
    if (isSolved(p))  return true;    // solved

    // Step 2: choose an unknown cell and guess
    auto [r, c] = findUnknown(p);
    auto saved = p.board;  // save current board state

    // Try filled
    p.board[r][c] = 'x';
    if (solve(p)) return true;
    p.board = saved;  // backtrack

    // Try empty
    p.board[r][c] = 'o';
    if (solve(p)) return true;
    p.board = saved;  // backtrack

    return false;  // both guesses lead to contradiction
}

// ==================== Main ====================
int main() {
    Nonogram puzzle;
    if (loadPuzzle("puzzle.txt", puzzle)) {
        cout << "Successfully loaded puzzle: " << puzzle.rows << "x" << puzzle.cols << endl;

        if (solve(puzzle))
            printBoard(puzzle);
        else
            cout << "Puzzle has no solution!" << endl;
    }
    return 0;
}