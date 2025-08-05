#!/bin/bash

# Script pour vérifier le status des Pull Requests
# Usage: ./check-pr-status.sh [pr_number]

set -e

# Couleurs pour l'affichage
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔍 Vérification du statut des Pull Requests${NC}"

# Si un numéro de PR est spécifié
if [ ! -z "$1" ]; then
    PR_NUMBER=$1
    echo -e "${BLUE}📊 Statut de la PR #$PR_NUMBER:${NC}"
    gh pr view $PR_NUMBER --json title,state,mergeable,statusCheckRollup,autoMergeRequest
else
    # Lister toutes les PR ouvertes
    echo -e "${BLUE}📋 Toutes les PR ouvertes:${NC}"
    gh pr list --json number,title,state,mergeable,statusCheckRollup,autoMergeRequest
fi

echo ""
echo -e "${BLUE}🤖 Configuration auto-merge du repo:${NC}"
gh api repos/:owner/:repo --jq '{
    allow_auto_merge: .allow_auto_merge,
    allow_squash_merge: .allow_squash_merge,
    allow_merge_commit: .allow_merge_commit,
    allow_rebase_merge: .allow_rebase_merge
}'
