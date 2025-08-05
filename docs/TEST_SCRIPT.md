# Test Script Usage

## ESPHome Impulse Cover Component Test Script

Ce script automatise les tests de validation et de compilation du composant Impulse Cover pour ESPHome.

### Utilisation

#### Test rapide (validation seulement)
```bash
./test-impulse-cover.sh
```

#### Test complet (validation + compilation)
```bash
./test-impulse-cover.sh --compile
```

### Ce que teste le script

#### Tests de Validation
- ✅ Configuration de base (`examples/basic-configuration.yaml`)
- ✅ Configuration avec capteurs (`examples/with-sensors.yaml`) 
- ✅ Configuration ouverture partielle (`examples/partial-test.yaml`)
- ✅ Présence des fichiers du composant
- ✅ Support conditionnel des binary sensors
- ✅ Documentation complète

#### Tests de Compilation (avec `--compile`)
- 🔨 Compilation configuration de base
- 🔨 Compilation configuration avec capteurs
- 🔨 Compilation configuration ouverture partielle

### Prérequis

- Python 3.x installé
- Git (pour cloner le dépôt)

### Configuration automatique

Le script configure automatiquement :
- Environment virtuel Python (`.venv`)
- Installation d'ESPHome
- Fichier `secrets.yaml` de test

### Résultats

Le script affiche :
- ✅ Tests réussis en vert
- ❌ Tests échoués en rouge
- 📋 Résumé complet à la fin
- 💡 Exemples d'utilisation
- 🔧 Conseils d'utilisation

### Exemple de sortie

```
=== ESPHome Impulse Cover Component Test ===
📦 Installing/updating ESPHome...
📋 ESPHome version: Version: 2025.7.4
🧪 Testing basic configuration validation...
✅ Basic configuration valid
🧪 Testing configuration with sensors validation...
✅ With sensors configuration valid
🧪 Testing partial opening configuration validation...
✅ Partial opening configuration valid
🔍 Checking component dependencies...
✅ Component __init__.py found
✅ Component cover.py found
✅ Component header file found
✅ Component implementation file found
🔍 Testing binary sensor conditional compilation...
✅ Binary sensor conditional compilation found
📚 Checking documentation...
✅ Partial opening documentation found
✅ README.md found
🎉 All tests passed successfully!
```

### Dépannage

Si un test échoue :
1. Le script affiche les détails de l'erreur
2. Vérifiez les prérequis
3. Vérifiez que tous les fichiers sont présents
4. Consultez les logs ESPHome pour plus de détails

### Integration CI/CD

Ce script peut être utilisé dans des pipelines CI/CD :

```yaml
# Exemple GitHub Actions
- name: Test Impulse Cover Component
  run: |
    chmod +x test-impulse-cover.sh
    ./test-impulse-cover.sh --compile
```
