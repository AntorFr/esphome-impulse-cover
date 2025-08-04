# Configuration suggérée pour les règles de protection de branche GitHub
# À configurer dans GitHub > Settings > Branches > Add rule

## Règles de protection pour la branche 'main'

### 1. Protection de base
- ✅ Require a pull request before merging
- ✅ Require approvals: 1
- ✅ Dismiss stale PR approvals when new commits are pushed
- ✅ Require review from code owners

### 2. Vérifications de statut requises
Les checks suivants doivent passer avant le merge :
- ✅ ESPHome Configuration Validation
- ✅ ESPHome Compilation Test  
- ✅ Code Quality Checks
- ✅ C++ Code Quality
- ✅ Documentation Check
- ✅ Security Scan
- ✅ ESPHome Standards Compliance

### 3. Restrictions supplémentaires
- ✅ Require branches to be up to date before merging
- ✅ Require conversation resolution before merging
- ✅ Restrict pushes that create files that match "**/*.secret"
- ✅ Restrict pushes that create files that match "**/password*"

### 4. Règles pour les administrateurs
- ✅ Include administrators (recommandé)
- ✅ Allow force pushes (uniquement pour les administrateurs si nécessaire)
- ✅ Allow deletions (uniquement pour les administrateurs)

### 5. Commande pour configurer via GitHub CLI (optionnel)
```bash
# Installer GitHub CLI si nécessaire: https://cli.github.com/
gh api repos/:owner/:repo/branches/main/protection \
  --method PUT \
  --field required_status_checks='{"strict":true,"checks":[{"context":"ESPHome Configuration Validation"},{"context":"ESPHome Compilation Test"},{"context":"Code Quality Checks"},{"context":"C++ Code Quality"},{"context":"Documentation Check"},{"context":"Security Scan"},{"context":"ESPHome Standards Compliance"}]}' \
  --field enforce_admins=true \
  --field required_pull_request_reviews='{"required_approving_review_count":1,"dismiss_stale_reviews":true}' \
  --field restrictions=null
```

### 6. Templates pour les Pull Requests
Créer un template `.github/pull_request_template.md` pour standardiser les PR.
