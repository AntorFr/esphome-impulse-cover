#!/bin/bash

# Script pour configurer l'environnement de développement ESPHome
# Usage: source setup_env.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_DIR="$SCRIPT_DIR/.venv"

echo "🚀 Configuration de l'environnement ESPHome..."

# Vérifier si Python3 est installé
if ! command -v python3 &> /dev/null; then
    echo "❌ Python3 n'est pas installé"
    exit 1
fi

# Créer l'environnement virtuel s'il n'existe pas
if [ ! -d "$VENV_DIR" ]; then
    echo "📦 Création de l'environnement virtuel..."
    python3 -m venv "$VENV_DIR"
fi

# Activer l'environnement virtuel
echo "🔧 Activation de l'environnement virtuel..."
source "$VENV_DIR/bin/activate"

# Vérifier si ESPHome est installé
if ! command -v esphome &> /dev/null; then
    echo "📥 Installation d'ESPHome..."
    pip install --upgrade pip
    pip install esphome
else
    echo "🔄 Vérification de la version d'ESPHome..."
    pip install --upgrade esphome
fi

echo "✅ Environnement prêt !"
echo "📍 ESPHome version: $(esphome version)"
echo ""
echo "🛠️  Commandes utiles :"
echo "  esphome config <file.yaml>     - Valider une configuration"
echo "  esphome compile <file.yaml>    - Compiler un firmware"
echo "  esphome upload <file.yaml>     - Uploader sur un ESP"
echo "  esphome logs <file.yaml>       - Voir les logs en temps réel"
echo ""
echo "📁 Fichiers de test disponibles :"
echo "  - simple-test.yaml     : Configuration basique"
echo "  - test-config.yaml     : Configuration avec capteurs"
echo "  - advanced-test.yaml   : Configuration avancée"
