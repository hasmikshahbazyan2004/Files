#include <iostream>
#include <vector>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class TreeNode {
public:
    std::string name;
    bool is_directory;
    std::vector<TreeNode*> children;

    TreeNode(std::string name, bool is_directory)
        : name(name), is_directory(is_directory) {}

    void addChild(TreeNode* child) {
        children.push_back(child);
    }

    void printTree(int depth = 0) const {
        for (int i = 0; i < depth; ++i) {
            std::cout << "    ";
        }
        std::cout << (is_directory ? "[DIR] " : "[FILE] ") << name << std::endl;

        for (const auto& child : children) {
            child->printTree(depth + 1);
        }
    }

    ~TreeNode() {
        for (auto child : children) {
            delete child;
        }
    }
};

TreeNode* buildFileTree(const fs::path& dir_path) {
    if (!fs::exists(dir_path)) {
        std::cerr << "Path does not exist!" << std::endl;
        return nullptr;
    }

    auto* root = new TreeNode(dir_path.filename().string(), fs::is_directory(dir_path));

    if (fs::is_directory(dir_path)) {
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            root->addChild(buildFileTree(entry.path()));
        }
    }

    return root;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory-path>" << std::endl;
        return 1;
    }

    fs::path root_path(argv[1]);
    TreeNode* root = buildFileTree(root_path);

    if (root) {
        root->printTree();
        delete root;
    }

    std::string name = "my_file_system.cpp";
    TreeNode child(name, 0);
    TreeNode* new_child = &child;
    root -> addChild(new_child);
    root = buildFileTree(root_path);
    return 0;
}
