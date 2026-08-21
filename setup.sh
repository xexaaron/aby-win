#!/usr/bin/env bash

set -e


CYAN="\e[36m"
GREEN="\e[32m"
YELLOW="\e[33m"
RESET="\e[0m"
GREY="\e[90m"
DARK_BLUE="\e[38;5;25m"

PROJECT_NAME="CxxTemplate"
NAMESPACE="aby"
DIR="cxx_template"



usage() {
    printf "${GREEN}usage${RESET}: ${YELLOW}%s${RESET} [${GREY}-ns${RESET}|${GREY}--namespace${RESET} <namespace>] [${GREY}-d${RESET}|${GREY}--dir${RESET} <project directory name>] [${GREY}-pn${RESET}|${GREY}--project-name${RESET} <name>] [${GREY}-h${RESET}|${GREY}--help${RESET}]\n" "$0"
    exit 1
}

if [[ $# -eq 0 ]]; then
    echo -e "${CYAN}Configure C++ Template${RESET}"

    read -rp "$(echo -e "${GREY}Project name ${RESET}[${GREEN}$PROJECT_NAME${RESET}]: ${YELLOW}")" input
    echo -ne "$RESET"
    [[ -n "$input" ]] && PROJECT_NAME="$input"

    read -rp "$(echo -e "${GREY}Project directory and name of executable ${RESET}[${GREEN}$DIR${RESET}]: ${YELLOW}")" input
    echo -ne "$RESET"
    [[ -n "$input" ]] && DIR="$input"

    read -rp "$(echo -e "${GREY}Namespace for preincluded files ${RESET}[${GREEN}$NAMESPACE${RESET}]: ${YELLOW}")" input
    echo -ne "$RESET"
    [[ -n "$input" ]] && NAMESPACE="$input"
else
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -ns|--namespace)
                NAMESPACE="$2"
                shift 2
                ;;
            -d|--dir)
                DIR="$2"
                shift 2
                ;;
            -pn|--project-name)
                PROJECT_NAME="$2"
                shift 2
                ;;
            -h|--help)
                usage
                ;;
            *)
                echo "Unknown option: $1"
                usage
                ;;
        esac
    done
fi

exec 3>&1 4>&2
exec >/dev/null 2>&1

# -----------------------------------------------------------------------------
# Project name
# -----------------------------------------------------------------------------

if [[ "$PROJECT_NAME" != "CxxTemplate" ]]; then
    sed -i \
        "s/project(CxxTemplate LANGUAGES CXX)/project(${PROJECT_NAME} LANGUAGES CXX)/" \
        CMakeLists.txt
  
    
fi

# -----------------------------------------------------------------------------
# Directory name
# -----------------------------------------------------------------------------

if [[ "$DIR" != "cxx_template" ]]; then
    mv cxx_template "$DIR"

    sed -i \
        "s/project(cxx_template)/project(${DIR})/" \
        "$DIR/CMakeLists.txt"

    if [[ -e ".vscode/c_cpp_properties.json" ]]; then
        sed -i \
            "s|\${workspaceFolder}/cxx_template/include|\${workspaceFolder}/${DIR}/include|g" \
            ".vscode/c_cpp_properties.json"
    fi

    sed -i \
        "s|add_subdirectory(\${CMAKE_CURRENT_SOURCE_DIR}/cxx_template)|add_subdirectory(\${CMAKE_CURRENT_SOURCE_DIR}/${DIR})|" \
        CMakeLists.txt
fi

# -----------------------------------------------------------------------------
# Namespace
# -----------------------------------------------------------------------------

if [[ "$NAMESPACE" != "aby" ]]; then
    find "$DIR" -type f \( \
        -name "*.hpp" -o \
        -name "*.h"   -o \
        -name "*.cpp" -o \
        -name "*.cc"  -o \
        -name "*.cxx" -o \
        -name "*.ipp" \
    \) -print0 | while IFS= read -r -d '' file; do
        sed -i \
            -e "s/\bnamespace[[:space:]]\+aby\b/namespace ${NAMESPACE}/g" \
            -e "s/\baby::/${NAMESPACE}::/g" \
            "$file"
    done
fi

printf "${GREEN}Template configured successfully.${RESET}\n" >&3

rm -rf .git

if command -v git >/dev/null 2>&1; then
    git init
    git add .
    git commit -m "Initial commit"
fi

if command -v gh >/dev/null 2>&1; then
    printf "${GREY}Create new github.com repository?${RESET} [${GREEN}n${RESET}] (y/n): ${YELLOW}" >&3
    read -r prompt
    printf "${RESET}" >&3
    prompt=${prompt,,}
    prompt=${prompt:-n}

    if [[ "$prompt" == "y" || "$prompt" == "yes" ]]; then
        printf "${GREY}Choose repo visibility${RESET} [${GREEN}private${RESET}] (private/public): ${YELLOW}" >&3
        read -r visibility
        printf "${RESET}" >&3
        visibility=${visibility,,}
        visibility=${visibility:-private}

        gh repo create "$PROJECT_NAME" \
            --source=. \
            --push \
            --"$visibility"

         USERNAME=$(gh api user --jq .login)
            printf "${GREEN}Created GitHub repository:${RESET} ${DARK_BLUE}https://github.com/%s/%s.git${RESET}\n" \
                "$USERNAME" "$PROJECT_NAME" >&3
    fi
fi

cat > temp_script.sh <<EOF
#!/usr/bin/env bash

sleep 1

mkdir -p ${PROJECT_NAME}/.vscode
mkdir -p ${PROJECT_NAME}/${DIR}/include
mkdir -p ${PROJECT_NAME}/${DIR}/src
mkdir -p ${PROJECT_NAME}/vendor
mkdir -p ${PROJECT_NAME}/.git

cp -r CxxTemplate/.vscode/* ${PROJECT_NAME}/.vscode
cp -r CxxTemplate/${DIR}/* ${PROJECT_NAME}/${DIR}
cp -r CxxTemplate/.git/* ${PROJECT_NAME}/.git 
cp CxxTemplate/.gitignore ${PROJECT_NAME}/.gitignore
cp CxxTemplate/.clang-format ${PROJECT_NAME}/.clang-format
cp CxxTemplate/CMakeLists.txt ${PROJECT_NAME}/CMakeLists.txt

rm -rf CxxTemplate

rm "\$0"
EOF

chmod +x temp_script.sh

mv temp_script.sh ../temp_script.sh

(
    cd ..
    ./temp_script.sh
) &

cd .. 
rm "$0"
rm -rf CxxTemplate
cd $PROJECT_NAME
