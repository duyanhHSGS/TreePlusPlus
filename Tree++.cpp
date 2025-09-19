#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

bool is_text(const fs::path& p) {
    ifstream in(p, ios::binary);
    if (!in) return false;
    char buf[1024];
    in.read(buf, sizeof(buf));
    auto n = in.gcount();
    for (int i=0; i<n; i++) {
        unsigned char c = buf[i];
        if ((c < 9 || c > 126) && c!='\n' && c!='\r' && c!='\t')
            return false;
    }
    return true;
}

void print_tree_collect(ofstream &out,
                        const fs::path& dir,
                        string prefix,
                        vector<fs::path>& textFiles) {
    vector<fs::directory_entry> dirs, files;
    for (auto& p : fs::directory_iterator(dir)) {
        if (p.is_directory()) dirs.push_back(p);
        else files.push_back(p);
    }
    sort(dirs.begin(), dirs.end(), [](auto&a,auto&b) {
        return a.path()<b.path();
    });
    sort(files.begin(), files.end(), [](auto&a,auto&b) {
        return a.path()<b.path();
    });

    for (size_t i=0; i<dirs.size(); ++i) {
        bool last = (i==dirs.size()-1 && files.empty());
        out << prefix << (last ? "└─" : "├─") << "[DIR] "
            << dirs[i].path().filename().string() << "\n";
        print_tree_collect(out, dirs[i].path(), prefix + (last ? "   " : "│  "), textFiles);
    }
    for (size_t i=0; i<files.size(); ++i) {
        bool last = (i==files.size()-1);
        const auto &f = files[i].path();
        out << prefix << (last ? "└─" : "├─")
            << f.filename().string() << "\n";
        if (is_text(f)) textFiles.push_back(f);
    }
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    fs::path base = (argc>1) ? argv[1] : ".";
    ofstream out("tree_plus_plus.txt");
    vector<fs::path> textFiles;

    out << "===== DIRECTORY TREE =====\n";
    print_tree_collect(out, base, "", textFiles);

    out << "\n===== TEXT CONTENT =====\n";
    string buf;
    for (auto &p : textFiles) {
        out << "\n===== " << p.string() << " =====\n";
        ifstream in(p, ios::binary);
        auto sz = fs::file_size(p);
        buf.resize(sz);
        in.read(buf.data(), sz);
        out.write(buf.data(), in.gcount());
        out << "\n";
    }
}
