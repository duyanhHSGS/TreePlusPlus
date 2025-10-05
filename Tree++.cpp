#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <ranges>

namespace fs = std::filesystem;

// ===================== CONFIG =====================
struct IgnoreConfig {
    std::vector<std::string> extensions = {
        ".exe", ".o", ".obj", ".slo", ".lo", ".gch", ".pch",
        ".dll", ".so", ".dylib", ".lib", ".a", ".lai", ".mod", ".smod",
        ".ilk", ".pdb", ".dwo",
        ".cbp", ".layout", ".depend"
    };

    std::vector<std::string> dir_prefixes = {
        "build", "CMakeFiles"
    };

    std::vector<std::string> dir_exact = {
        ".git"   // ignore .git folder
    };

    std::vector<std::string> file_exact = {
        "tree_plus_plus.txt" // ignore output file itself
    };
};

// ===================== IGNORE LOGIC =====================
bool should_ignore(const fs::directory_entry& entry, const IgnoreConfig& cfg) {
    const auto& p = entry.path();
    const std::string fname = p.filename().string();

    // Ignore directories by prefix
    if (entry.is_directory()) {
        for (auto const& d : cfg.dir_prefixes) {
            if (fname.starts_with(d)) return true;
        }
        for (auto const& d : cfg.dir_exact) {
            if (fname == d) return true;
        }
    }

    // Ignore files by exact name
    if (entry.is_regular_file()) {
        for (auto const& f : cfg.file_exact) {
            if (fname == f) return true;
        }
    }

    // Ignore by extension
    const std::string ext = p.extension().string();
    return std::ranges::any_of(cfg.extensions,
                               [&](auto const& e){ return e == ext; });
}

// ===================== TEXT DETECTION =====================
bool is_text_file(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return false;

    char buf[1024];
    in.read(buf, sizeof(buf));
    auto n = in.gcount();

    for (int i = 0; i < n; i++) {
        unsigned char c = buf[i];
        if ((c < 9 || c > 126) && c != '\n' && c != '\r' && c != '\t')
            return false;
    }
    return true;
}

// ===================== TREE WALKER =====================
void print_tree_collect(std::ostream& out,
                        const fs::path& dir,
                        std::string prefix,
                        std::vector<fs::path>& textFiles,
                        const IgnoreConfig& cfg)
{
    std::vector<fs::directory_entry> dirs, files;

    for (auto const& entry : fs::directory_iterator(dir)) {
        if (should_ignore(entry, cfg)) continue;
        if (entry.is_directory()) dirs.push_back(entry);
        else files.push_back(entry);
    }

    auto sorter = [](auto const& a, auto const& b) {
        return a.path().filename().string() < b.path().filename().string();
    };
    std::ranges::sort(dirs, sorter);
    std::ranges::sort(files, sorter);

    for (size_t i = 0; i < dirs.size(); ++i) {
        bool last = (i == dirs.size()-1 && files.empty());
        out << prefix << (last ? "└─" : "├─") << "[DIR] "
            << dirs[i].path().filename().string() << "\n";
        print_tree_collect(out, dirs[i].path(),
                           prefix + (last ? "   " : "│  "),
                           textFiles, cfg);
    }

    for (size_t i = 0; i < files.size(); ++i) {
        bool last = (i == files.size()-1);
        const auto& f = files[i].path();
        out << prefix << (last ? "└─" : "├─") << f.filename().string() << "\n";
        if (is_text_file(f)) textFiles.push_back(f);
    }
}

// ===================== MAIN =====================
int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    fs::path base = (argc > 1) ? argv[1] : ".";
    fs::path self_file = fs::absolute(argv[0]);

    IgnoreConfig cfg;
    std::ofstream out("tree_plus_plus.txt");
    std::vector<fs::path> textFiles;
    out << "===== My Tree++ =====\n";
    out << "This crap prints out trees, and ignores the bin trash\n";
    out << "It is optimized for c++ project!\n";
    out << "===== DIRECTORY TREE =====\n";
    print_tree_collect(out, base, "", textFiles, cfg);

    out << "\n===== TEXT CONTENT =====\n";
    std::string buf;
    for (auto const& p : textFiles) {
        if (fs::absolute(p) == self_file) continue;

        out << "\n===== " << p.string() << " =====\n";
        std::ifstream in(p, std::ios::binary);
        if (!in) continue;

        auto sz = fs::file_size(p);
        buf.resize(sz);
        in.read(buf.data(), sz);
        out.write(buf.data(), in.gcount());
        out << "\n";
    }
}
