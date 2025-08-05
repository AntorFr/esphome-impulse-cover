#!/bin/bash

# Script pour créer une PR avec auto-merge activé
# Usage: ./create-pr.sh [titre] [description]

set -e s

# Couleurs pour l'affichage
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚀 Création automatique d'une Pull Request${NC}"

# Vérifier qu'on est sur la branche dev
CURRENT_BRANCH=$(git branch --show-current)
if [ "$CURRENT_BRANCH" != "dev" ]; then
    echo -e "${RED}❌ Erreur: Vous devez être sur la branche 'dev' pour créer une PR${NC}"
    echo -e "${YELLOW}💡 Conseil: git checkout dev${NC}"
    exit 1
fi

# Vérifier qu'il n'y a pas de changements non commitées
if [[ -n $(git status --porcelain) ]]; then
    echo -e "${RED}❌ Erreur: Il y a des changements non commitées${NC}"
    echo -e "${YELLOW}💡 Conseil: Committez vos changements avant de créer la PR${NC}"
    exit 1
fi

# Pousser les derniers changements
echo -e "${BLUE}📤 Push des derniers changements...${NC}"
git push origin dev

# Paramètres par défaut
DEFAULT_TITLE="🤖 Add Auto-Merge System - v1.0.0-beta3"
DEFAULT_BODY="## 🚀 Auto-Merge System Release

### ✨ New Features
- **Complete auto-merge automation system**
- **Intelligent PR creation and management** 
- **GitHub CLI integration** for seamless workflow
- **Automated CI/CD pipeline integration**

### 🤖 Automation Tools
- \`create-pr.sh\`: Automated PR creation with auto-merge
- \`check-pr-status.sh\`: PR status monitoring and validation  
- \`auto-merge.yml\`: GitHub Actions workflow for merge automation

### 🔧 Technical Improvements
- Repository-level auto-merge configuration
- Error handling and safety validations
- Color-coded terminal interface
- Comprehensive status reporting

### 📈 Developer Experience
- Zero-friction PR workflow
- Automatic merge on green CI/CD
- Enhanced productivity tools
- Complete automation pipeline

**Ready for production deployment!** 🚀"

TITLE=${1:-$DEFAULT_TITLE}
BODY=${2:-$DEFAULT_BODY}

# Créer la PR avec auto-merge
echo -e "${BLUE}📝 Création de la Pull Request...${NC}"
PR_URL=$(gh pr create \
    --title "$TITLE" \
    --body "$BODY" \
    --base main \
    --head dev)

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ Pull Request créée: $PR_URL${NC}"
    
    # Extraire le numéro de PR de l'URL
    PR_NUMBER=$(echo "$PR_URL" | grep -o '[0-9]*$')
    
    # Activer l'auto-merge
    echo -e "${BLUE}🤖 Activation de l'auto-merge...${NC}"
    if gh pr merge "$PR_NUMBER" --auto --squash; then
        echo -e "${GREEN}✅ Auto-merge activé! La PR sera automatiquement mergée quand toutes les vérifications passeront.${NC}"
    else
        echo -e "${YELLOW}⚠️  Impossible d'activer l'auto-merge. Vous pouvez le faire manuellement sur GitHub.${NC}"
    fi
    
    echo -e "${GREEN}🎉 Processus terminé avec succès!${NC}"
    echo -e "${BLUE}🔗 Lien vers la PR: $PR_URL${NC}"
else
    echo -e "${RED}❌ Erreur lors de la création de la PR${NC}"
    exit 1
fi
