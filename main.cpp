#include <cstdint>
#include <climits>

#include <chrono>
#include <iostream>
#include <utility>
#include <vector>
#include <queue>


double time() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count()
        / 1e9;
}


struct my_graph {
    std::vector<int> neighbours;
    std::vector<int> offset;

    // offset[n] should be equal to neighbours.size()
    size_t size() const {
        return offset.size() - 1;
    }
};


void generate_graph(int ni, int nj, my_graph &ans) {
    // neighbours of i-th vertex have indices offset[i]..offset[i+1]-1
    ans.offset.reserve(ni * nj);
    ans.neighbours.reserve(ni*nj * 4 - 2 * (ni + nj));
    // this is rectilinear grid
    for (int i = 0; i < ni; ++i) {
        for (int j = 0; j < nj; ++j) {
            ans.offset.push_back(ans.neighbours.size());
            int I = j + i * nj;
            if (j > 0) ans.neighbours.push_back(I-1);
            if (j < nj - 1) ans.neighbours.push_back(I+1);
            if (i < ni - 1) ans.neighbours.push_back(I+nj);
            if (i > 0) ans.neighbours.push_back(I-nj);
            //if (i < ni - 1 && j < nj - 1) ans.neighbours.push_back(I+nj+1);
            //if (i > 0 && j > 0) ans.neighbours.push_back(I-nj-1);
        }
    }
    ans.offset.push_back(ans.neighbours.size());
}


bool check(int i, int c, const my_graph &gr, const std::vector<int> &colors) {
    // check if i-th vertex has no neighbours with color c
    for (int j = gr.offset[i]; j < gr.offset[i+1]; ++j) {
        int k = gr.neighbours[j];
        if (colors[k] == c) {
            return false;
        }
    }
    return true;
}

int color_graph(const my_graph &graph, std::vector<int> &ans) {
    // breadth-first search
    ans.resize(graph.size(), -1);

    std::vector<bool> used(graph.size(), false);
    std::queue<int> queue;
    int colors_used = 1;

    queue.push(0);
    while (queue.size()) {
        int i = queue.front();
        queue.pop();
        if (used[i]) {
            continue;
        }
        used[i] = true;
        // find first suitable color
        for (int c = 0; c < colors_used; ++c) {
            if (check(i, c, graph, ans)) {
                ans[i] = c;
                break;
            }
        }
        // if not found, get new color
        if (ans[i] < 0) {
            ans[i] = colors_used++;
        }
        for (int j = graph.offset[i]; j < graph.offset[i+1]; ++j) {
            if (!used[graph.neighbours[j]]) {
                queue.push(graph.neighbours[j]);
            }
        }
    }

    return colors_used;
}


// this program works only if nx * ny < INT_MAX
int main(int argc, char **args) {
    if (argc <= 1) {
        std::cout << "Usage: " << args[0] << " --nx=NX --ny=NY" << std::endl;
        return 1;
    }
    int nx = -1, ny = -1;
    for (int i = 1; i < argc; ++i) {
        std::string arg(args[i]);
        if (arg.find("--nx=") == 0) {
            nx = std::stoi(arg.substr(3+2));
        }
        else if (arg.find("--ny=") == 0) {
            ny = std::stoi(arg.substr(3+2));
        }
    }

    if (nx < 1 || ny < 1) {
        std::cerr << "nx < 1 || ny < 1" << std::endl;
        return 2;
    }
    if (sizeof(int) * CHAR_BIT != 32) {
        std::cerr << "What is going on?? int is not 32-bit" << std::endl;
        return 2;
    }
    if ((nx * (uint64_t)ny) > INT_MAX) {
        std::cerr << "nx * ny > INT_MAX\n";
        return 2;
    }

    //

    // https://vtk.org/wp-content/uploads/2015/04/file-formats.pdf
    std::cout << "# vtk DataFile Version 2.0\n"
        "some rectangular colored grid\n"
        "ASCII\n"
        "DATASET UNSTRUCTURED_GRID\n\n";

    my_graph graph;
    generate_graph(nx, ny, graph);

    std::vector<int> colors;
    int colors_used = color_graph(graph, colors);
    {
        // stats about colors
        std::cerr << "colors_used = " << colors_used << "\n";
        std::vector<int> used(colors_used);
        for (int c : colors) {
            ++used[c];
        }
        std::cerr << "used = [";
        for (int c : used) {
            std::cerr << c << ", ";
        }
        std::cerr << "]\n";
    }

    std::cout << "POINTS " << colors.size() << " float\n";
    // graph.size() < INT_MAX, see comment before main
    for (int i = 0; i < (int)graph.size(); ++i) {
        std::cout << i / ny << " " << i % ny << " " << 0 << "\n";
    }

    std::cout << "\nCELLS " << (graph.neighbours.size() / 2)
        << " " << 3 * (graph.neighbours.size() / 2) << "\n";
    for (int i = 0; i < (int)graph.size(); ++i) {
        for (int j = graph.offset[i]; j < graph.offset[i+1]; ++j) {
            auto k = graph.neighbours[j];
            if (i > k) {
                std::cout << 2 << " " << i << " " << k << "\n";
            }
        }
    }

    std::cout << "\nCELL_TYPES " << (graph.neighbours.size() / 2) << "\n";
    for (int i = 0; i < (int)(graph.neighbours.size() / 2); ++i) {
        std::cout << "3\n";
    }

    std::cout << "\n\nPOINT_DATA " << colors.size()
        << "\nSCALARS Color float 1\nLOOKUP_TABLE default\n";
    for (int c : colors) {
        std::cout << c << "\n";
    }
}
