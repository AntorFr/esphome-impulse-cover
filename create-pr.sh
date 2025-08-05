#!/bin/bash

# 🚀 ESPHome Impulse Cover - PR Creation Helper
# This script helps create a pull request from dev to main

set -e

echo "🚀 === PREPARING PULL REQUEST ==="
echo

# Check current branch
CURRENT_BRANCH=$(git branch --show-current)
echo "📍 Current branch: $CURRENT_BRANCH"

if [ "$CURRENT_BRANCH" != "dev" ]; then
    echo "⚠️  Warning: You're not on the 'dev' branch"
    echo "🔄 Switching to dev branch..."
    git checkout dev
fi

# Update branches
echo "🔄 Updating branches..."
git fetch origin

# Show commit summary
echo
echo "📋 === COMMITS TO BE MERGED ==="
git log --oneline origin/main..dev
echo

# Show file changes summary
echo "📊 === FILES CHANGED ==="
git diff origin/main..dev --stat
echo

# Run final validation
echo "🧪 === FINAL VALIDATION ==="
if [ -f "./pre-commit-check.sh" ]; then
    echo "🔍 Running pre-commit validation..."
    ./pre-commit-check.sh
    
    if [ $? -eq 0 ]; then
        echo "✅ All validations passed!"
    else
        echo "❌ Validation failed. Please fix issues before creating PR."
        exit 1
    fi
else
    echo "⚠️  Pre-commit script not found, skipping validation"
fi

echo
echo "🎯 === PR INFORMATION ==="
echo "Source branch: dev"
echo "Target branch: main"
echo "Tag: v1.0.0-beta2"
echo "Files changed: 11 files (+552, -146 lines)"
echo

echo "📋 === NEXT STEPS ==="
echo "1. Review the PR template: PR_TEMPLATE.md"
echo "2. Create PR on GitHub:"
echo "   - Go to: https://github.com/AntorFr/esphome-impulse-cover"
echo "   - Click 'New Pull Request'"
echo "   - Select: dev → main"
echo "   - Use PR_TEMPLATE.md content as description"
echo "3. Add reviewers and labels as needed"
echo
echo "🎉 Ready to create pull request!"
