#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>

namespace file = std::filesystem;

template <class T>
class TreeNode {
private:
    T m_name;
    bool is_directory;
    bool is_symlink;
    std::vector<TreeNode*> children;
    file::path node_path;  

public:
    TreeNode(T name, bool directory, bool symbolic_link, const file::path& path) 
        : m_name(name), is_directory(directory), is_symlink(symbolic_link), node_path(path) {}

    ~TreeNode() {
        for (const auto& child : children) {
            delete child;
        }
    }

    void addChild(TreeNode* child) {
        children.push_back(child);
    }

    void printTree(int depth = 0, int depth_limit = -1) const {
        if (depth_limit != -1 && depth_limit < depth){
            return;
        }
        for (int i = 0; i < depth; ++i) {
            std::cout << "   ";
        }

        std::cout << (is_symlink ? "[SYMLINK]" : (is_directory ? "[DIR] " : "[FILE] ")) << m_name;

        if (!is_directory && !is_symlink) {
            try {
                auto file_size = file::file_size(node_path);
                auto last_mod_time = file::last_write_time(node_path);

                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    last_mod_time - file::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

                std::cout << ": SIZE -> " << file_size << " bytes, "
                          << "LAST MODIFIED -> " << std::asctime(std::localtime(&cftime));
            } catch (const file::filesystem_error& e) {
                std::cerr << "Error accessing file metadata: " << e.what() << std::endl;
            }
        }

        std::cout << std::endl;
        for (const auto& child : children) {
            child->printTree(depth + 1, depth_limit);
        }
    }
};

template <typename T>
TreeNode<T>* buildfileSystem(const file::path& path, int current_depth = 0, int depth_limit = -1) {

    if (depth_limit != -1 && depth_limit < current_depth){
        return nullptr;
    }
    try {
        if (!file::exists(path) && !file::is_symlink(path)) {
            std::cerr << "The provided path doesn't exist!\n";
            return nullptr;
        }

        bool is_dir = file::is_directory(path);
        bool is_link = file::is_symlink(path);

        auto* root = new TreeNode<T>(path.filename().string(), is_dir, is_link, path);

      
        if (is_dir || (is_link && file::is_directory(file::read_symlink(path)))) {
            for (const auto& i : file::directory_iterator(path)) {
                    root -> addChild(buildfileSystem<T>(i.path(), current_depth + 1, depth_limit));
                
            }
        }
        return root;
    } catch (file::filesystem_error& error) {
        std::cerr << "Error when accessing " << path << " (" << error.what() << ")" << std::endl;
        return nullptr;
        }
    
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "No path was provided\n";
        return 1;
    }

   
    file::path root_path(argv[1]);
    int depth = (argc >= 3) ? std::atoi(argv[2]) : 0;
    int depth_limit = (argc >= 4) ? std::atoi(argv[3]) : -1;
    TreeNode<std::string>* root = buildfileSystem<std::string>(root_path, 0, depth_limit);

    if (!root) {
        std::cerr << "Failed to build the file system tree\n";
        return 1;
    }
    

    root->printTree(depth, depth_limit);
    delete root;

    return 0;
}
