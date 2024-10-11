sudo apt install micro clangd gh -y
sudo snap install code --classic
code --install-extension llvm-vs-code-extensions.vscode-clangd
code --install-extension 13xforever.language-x86-64-assembly

gh auth login -w
gh config set -h github.com git_protocol https