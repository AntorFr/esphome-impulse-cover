#!/bin/bash

# ESPHome Impulse Cover Component Test Script
# Ce script automatise les tests de validation et de compilation
# Utilisé par les GitHub Actions workflows

set -e  # Arrêter en cas d'erreur

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Variables
COMPILE_MODE=false
VERBOSE=false
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Fonction d'aide
show_help() {
    echo -e "${BLUE}=== ESPHome Impulse Cover Component Test Script ===${NC}"
    echo -e "${CYAN}Usage: $0 [OPTIONS]${NC}"
    echo ""
    echo "Options:"
    echo "  --compile    Effectue également la compilation complète"
    echo "  --verbose    Affichage verbeux"
    echo "  --help       Affiche cette aide"
    echo ""
    echo "Exemples:"
    echo "  $0                    # Test de validation seulement"
    echo "  $0 --compile          # Test complet avec compilation"
    echo "  $0 --compile --verbose # Test complet avec logs détaillés"
}

# Traitement des arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --compile)
            COMPILE_MODE=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo -e "${RED}❌ Option inconnue: $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

# Fonction pour afficher les logs en mode verbeux
log_verbose() {
    if [ "$VERBOSE" = true ]; then
        echo -e "${CYAN}[VERBOSE] $1${NC}"
    fi
}

# Fonction pour tester une configuration
test_config() {
    local config_file=$1
    local config_name=$(basename "$config_file" .yaml)
    local test_type=$2  # "validate" ou "compile"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ "$test_type" = "compile" ]; then
        echo -e "${BLUE}🔨 Compilation de $config_name...${NC}"
        if [ "$VERBOSE" = true ]; then
            esphome compile "$config_file"
        else
            esphome compile "$config_file" &>/dev/null
        fi
    else
        echo -e "${BLUE}📋 Validation de $config_name...${NC}"
        if [ "$VERBOSE" = true ]; then
            esphome config "$config_file"
        else
            esphome config "$config_file" &>/dev/null
        fi
    fi
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ $config_name: RÉUSSI${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo -e "${RED}❌ $config_name: ÉCHEC${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

# Fonction pour vérifier les prérequis
check_prerequisites() {
    echo -e "${BLUE}🔍 Vérification des prérequis...${NC}"
    
    # Vérifier ESPHome
    if ! command -v esphome &> /dev/null; then
        echo -e "${YELLOW}📦 Installation d'ESPHome...${NC}"
        pip install esphome>=2023.12.0
    fi
    
    # Afficher la version ESPHome
    ESPHOME_VERSION=$(esphome version 2>/dev/null | grep "Version" | cut -d: -f2 | xargs)
    echo -e "${GREEN}📋 ESPHome version: $ESPHOME_VERSION${NC}"
    
    # Vérifier la structure du projet
    if [ ! -d "components/impulse_cover" ]; then
        echo -e "${RED}❌ Dossier du composant non trouvé${NC}"
        exit 1
    fi
    
    if [ ! -d "examples" ]; then
        echo -e "${RED}❌ Dossier 'examples' non trouvé${NC}"
        exit 1
    fi
    
    # Créer secrets.yaml si nécessaire
    if [ ! -f "examples/secrets.yaml" ]; then
        log_verbose "Création du fichier secrets.yaml de test"
        cat > examples/secrets.yaml << EOF
# Fichier de secrets temporaire pour test
wifi_ssid: "test-network"
wifi_password: "test1234"
ap_password: "test1234"
api_encryption: "test1234567890123456789012345678901234567890123456789012345678901234"
ota_password: "test1234"
EOF
    fi
}

# Fonction pour tester la structure du composant
test_component_structure() {
    echo -e "${BLUE}🔍 Test de la structure du composant...${NC}"
    
    local files=(
        "components/impulse_cover/__init__.py"
        "components/impulse_cover/cover.py"
        "components/impulse_cover/impulse_cover.h"
        "components/impulse_cover/impulse_cover.cpp"
    )
    
    for file in "${files[@]}"; do
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        if [ -f "$file" ]; then
            echo -e "${GREEN}✅ $(basename "$file") trouvé${NC}"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo -e "${RED}❌ $(basename "$file") manquant${NC}"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    done
    
    # Vérifier le support conditionnel des binary sensors
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    if grep -q "#ifdef USE_BINARY_SENSOR" components/impulse_cover/impulse_cover.cpp; then
        echo -e "${GREEN}✅ Support conditionnel binary sensor trouvé${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}❌ Support conditionnel binary sensor manquant${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
}

# Fonction pour tester la documentation
test_documentation() {
    echo -e "${BLUE}📚 Test de la documentation...${NC}"
    
    local docs=(
        "README.md"
        "manifest.json"
        "VERSION"
    )
    
    for doc in "${docs[@]}"; do
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        if [ -f "$doc" ]; then
            echo -e "${GREEN}✅ $doc trouvé${NC}"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo -e "${RED}❌ $doc manquant${NC}"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    done
}

# Fonction principale
main() {
    echo -e "${BLUE}=== ESPHome Impulse Cover Component Test ===${NC}"
    echo ""
    
    # Vérification des prérequis
    check_prerequisites
    echo ""
    
    # Test de la structure du composant
    test_component_structure
    echo ""
    
    # Test de la documentation
    test_documentation
    echo ""
    
    # Test des configurations d'exemple
    echo -e "${BLUE}🧪 Test des configurations d'exemple...${NC}"
    
    local configs=(
        "examples/basic-configuration.yaml"
        "examples/with-sensors.yaml"
        "examples/partial-test.yaml"
        "examples/esp8266-basic.yaml"
    )
    
    # Tests de validation
    for config in "${configs[@]}"; do
        if [ -f "$config" ]; then
            test_config "$config" "validate"
        else
            echo -e "${YELLOW}⏭️  Ignoré: $(basename "$config") (fichier non trouvé)${NC}"
        fi
    done
    
    # Tests de compilation si demandé
    if [ "$COMPILE_MODE" = true ]; then
        echo ""
        echo -e "${BLUE}🔨 Test de compilation complète...${NC}"
        
        for config in "${configs[@]}"; do
            if [ -f "$config" ]; then
                # Ignorer secrets.yaml
                if [[ $(basename "$config") != "secrets.yaml" ]]; then
                    test_config "$config" "compile"
                fi
            fi
        done
    fi
    
    # Affichage des résultats
    echo ""
    echo -e "${BLUE}📊 RÉSULTATS DES TESTS${NC}"
    echo -e "${BLUE}======================${NC}"
    echo -e "Total: ${CYAN}$TOTAL_TESTS${NC}"
    echo -e "Réussis: ${GREEN}$PASSED_TESTS${NC}"
    echo -e "Échecs: ${RED}$FAILED_TESTS${NC}"
    echo ""
    
    if [ $FAILED_TESTS -eq 0 ]; then
        echo -e "${GREEN}🎉 TOUS LES TESTS SONT PASSÉS !${NC}"
        echo -e "${GREEN}✅ Le composant Impulse Cover est prêt${NC}"
        
        if [ "$COMPILE_MODE" = true ]; then
            echo -e "${GREEN}🔨 Compilation complète réussie${NC}"
        else
            echo -e "${YELLOW}💡 Lancez avec --compile pour tester la compilation${NC}"
        fi
        
        exit 0
    else
        echo -e "${RED}❌ $FAILED_TESTS test(s) ont échoué${NC}"
        echo -e "${RED}🔧 Veuillez corriger les erreurs avant de continuer${NC}"
        
        if [ "$VERBOSE" = false ]; then
            echo -e "${YELLOW}💡 Lancez avec --verbose pour plus de détails${NC}"
        fi
        
        exit 1
    fi
}

# Exécution du script principal
main "$@"
