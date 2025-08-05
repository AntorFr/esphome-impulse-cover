#!/bin/bash

# Script de test de qualité du code pour ESPHome Impulse Cover
# Usage: ./quality_check.sh

set -e

echo "🔍 Vérification de la qualité du code ESPHome Impulse Cover"
echo "============================================================"

# Couleurs pour les messages
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Fonction pour afficher les résultats
print_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✅ $2${NC}"
    else
        echo -e "${RED}❌ $2${NC}"
        FAILED_CHECKS=$((FAILED_CHECKS + 1))
    fi
}

FAILED_CHECKS=0

# 1. Validation des configurations ESPHome
echo -e "\n${BLUE}📋 1. Validation des configurations ESPHome${NC}"
if command -v esphome >/dev/null 2>&1; then
    for config in simple-test.yaml test-config.yaml advanced-test.yaml clean-test.yaml; do
        if [ -f "$config" ]; then
            echo "Validation de $config..."
            if esphome config "$config" >/dev/null 2>&1; then
                print_result 0 "Configuration $config valide"
            else
                print_result 1 "Configuration $config invalide"
            fi
        fi
    done
else
    print_result 1 "ESPHome non installé (utilisez: pip install esphome)"
fi

# 2. Vérification du formatage Python
echo -e "\n${BLUE}🐍 2. Formatage du code Python${NC}"
if command -v black >/dev/null 2>&1; then
    if black --check components/ >/dev/null 2>&1; then
        print_result 0 "Formatage Python correct"
    else
        print_result 1 "Formatage Python incorrect (utilisez: black components/)"
    fi
else
    print_result 1 "Black non installé (utilisez: pip install black)"
fi

# 3. Vérification du tri des imports
echo -e "\n${BLUE}📦 3. Tri des imports Python${NC}"
if command -v isort >/dev/null 2>&1; then
    if isort --check-only components/ >/dev/null 2>&1; then
        print_result 0 "Imports Python triés correctement"
    else
        print_result 1 "Imports Python mal triés (utilisez: isort components/)"
    fi
else
    print_result 1 "isort non installé (utilisez: pip install isort)"
fi

# 4. Linting Python
echo -e "\n${BLUE}🔍 4. Linting du code Python${NC}"
if command -v flake8 >/dev/null 2>&1; then
    if flake8 components/ >/dev/null 2>&1; then
        print_result 0 "Code Python conforme aux standards"
    else
        print_result 1 "Problèmes de style Python détectés"
        echo "Détails:"
        flake8 components/
    fi
else
    print_result 1 "flake8 non installé (utilisez: pip install flake8)"
fi

# 5. Formatage du code C++
echo -e "\n${BLUE}⚙️  5. Formatage du code C++${NC}"
if command -v clang-format >/dev/null 2>&1; then
    cpp_files=$(find components/ -name "*.cpp" -o -name "*.h" 2>/dev/null)
    if [ -n "$cpp_files" ]; then
        if echo "$cpp_files" | xargs clang-format --dry-run --Werror >/dev/null 2>&1; then
            print_result 0 "Formatage C++ correct"
        else
            print_result 1 "Formatage C++ incorrect"
            echo "Pour corriger: find components/ -name '*.cpp' -o -name '*.h' | xargs clang-format -i"
        fi
    else
        print_result 0 "Aucun fichier C++ trouvé"
    fi
else
    print_result 1 "clang-format non installé"
fi

# 6. Analyse statique C++
echo -e "\n${BLUE}🔍 6. Analyse statique C++${NC}"
if command -v cppcheck >/dev/null 2>&1; then
    if cppcheck --enable=warning,style,performance,portability --error-exitcode=1 components/ >/dev/null 2>&1; then
        print_result 0 "Analyse statique C++ réussie"
    else
        print_result 1 "Problèmes détectés par l'analyse statique C++"
    fi
else
    print_result 1 "cppcheck non installé"
fi

# 7. Validation YAML
echo -e "\n${BLUE}📝 7. Validation des fichiers YAML${NC}"
if command -v yamllint >/dev/null 2>&1; then
    if yamllint *.yaml *.yml >/dev/null 2>&1; then
        print_result 0 "Fichiers YAML valides"
    else
        print_result 1 "Erreurs dans les fichiers YAML"
    fi
else
    print_result 1 "yamllint non installé (utilisez: pip install yamllint)"
fi

# 8. Vérification de la structure
echo -e "\n${BLUE}🏗️  8. Structure du composant ESPHome${NC}"
required_files=("components/impulse_cover/__init__.py" "components/impulse_cover/cover.py" "components/impulse_cover/impulse_cover.h" "components/impulse_cover/impulse_cover.cpp")
for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        print_result 0 "Fichier requis trouvé: $(basename $file)"
    else
        print_result 1 "Fichier requis manquant: $file"
    fi
done

# 9. Vérification de la documentation
echo -e "\n${BLUE}📚 9. Documentation${NC}"
if [ -f "README.md" ]; then
    print_result 0 "README.md présent"
    
    # Vérifier les sections importantes
    sections=("Installation" "Configuration" "Usage" "Example")
    for section in "${sections[@]}"; do
        if grep -qi "$section" README.md; then
            print_result 0 "Section '$section' trouvée dans README"
        else
            print_result 1 "Section '$section' manquante dans README"
        fi
    done
else
    print_result 1 "README.md manquant"
fi

# Résumé final
echo -e "\n${BLUE}📊 Résumé de la vérification${NC}"
echo "============================================"

if [ $FAILED_CHECKS -eq 0 ]; then
    echo -e "${GREEN}🎉 Tous les tests de qualité sont passés !${NC}"
    echo "✅ Le code est prêt pour un Pull Request"
    exit 0
else
    echo -e "${RED}❌ $FAILED_CHECKS vérification(s) ont échoué${NC}"
    echo "🔧 Veuillez corriger les problèmes avant de soumettre un PR"
    exit 1
fi
