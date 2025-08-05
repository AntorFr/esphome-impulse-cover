#!/bin/bash

# CI/CD Validation Script for ESPHome Impulse Cover
# This script validates GitHub Actions workflows and configurations

set -e

echo "=== CI/CD Configuration Validation ==="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    if [ "$status" = "OK" ]; then
        echo -e "${GREEN}✅ $message${NC}"
    elif [ "$status" = "WARNING" ]; then
        echo -e "${YELLOW}⚠️ $message${NC}"
    else
        echo -e "${RED}❌ $message${NC}"
    fi
}

# Check if we're in the right directory
if [ ! -f "test-impulse-cover.sh" ]; then
    print_status "ERROR" "Not in project root directory"
    exit 1
fi

echo "🔍 Checking GitHub Actions workflows..."

# Check workflow files exist
workflows_dir=".github/workflows"
if [ ! -d "$workflows_dir" ]; then
    print_status "ERROR" "GitHub workflows directory not found"
    exit 1
fi

# Check individual workflow files
workflows=("quality-check.yml" "release.yml" "maintenance.yml" "performance.yml")
for workflow in "${workflows[@]}"; do
    if [ -f "$workflows_dir/$workflow" ]; then
        print_status "OK" "Workflow $workflow found"
    else
        print_status "ERROR" "Workflow $workflow not found"
    fi
done

echo ""
echo "🔍 Checking GitHub templates..."

# Check issue templates
if [ -d ".github/ISSUE_TEMPLATE" ]; then
    templates=("bug_report.yml" "feature_request.yml" "question.yml")
    for template in "${templates[@]}"; do
        if [ -f ".github/ISSUE_TEMPLATE/$template" ]; then
            print_status "OK" "Issue template $template found"
        else
            print_status "ERROR" "Issue template $template not found"
        fi
    done
else
    print_status "ERROR" "Issue templates directory not found"
fi

# Check PR template
if [ -f ".github/PULL_REQUEST_TEMPLATE.md" ]; then
    print_status "OK" "Pull request template found"
else
    print_status "ERROR" "Pull request template not found"
fi

echo ""
echo "🔍 Checking configuration files..."

# Check Dependabot
if [ -f ".github/dependabot.yml" ]; then
    print_status "OK" "Dependabot configuration found"
else
    print_status "WARNING" "Dependabot configuration not found"
fi

# Check pyproject.toml
if [ -f "pyproject.toml" ]; then
    print_status "OK" "pyproject.toml found"
    
    # Check for key sections
    if grep -q "\[tool.black\]" pyproject.toml; then
        print_status "OK" "Black configuration found"
    else
        print_status "WARNING" "Black configuration not found"
    fi
    
    if grep -q "\[tool.isort\]" pyproject.toml; then
        print_status "OK" "isort configuration found"
    else
        print_status "WARNING" "isort configuration not found"
    fi
    
    if grep -q "\[tool.pylint" pyproject.toml; then
        print_status "OK" "Pylint configuration found"
    else
        print_status "WARNING" "Pylint configuration not found"
    fi
else
    print_status "WARNING" "pyproject.toml not found"
fi

echo ""
echo "🔍 Checking development environment..."

# Check devcontainer
if [ -f ".devcontainer/devcontainer.json" ]; then
    print_status "OK" "Dev container configuration found"
else
    print_status "WARNING" "Dev container configuration not found"
fi

# Check VS Code workspace
if [ -f "esphome-impulse-cover.code-workspace" ]; then
    print_status "OK" "VS Code workspace configuration found"
else
    print_status "WARNING" "VS Code workspace configuration not found"
fi

echo ""
echo "🔍 Checking documentation..."

# Check documentation files
docs=("CONTRIBUTING.md" "SECURITY.md" "docs/DEVELOPER.md")
for doc in "${docs[@]}"; do
    if [ -f "$doc" ]; then
        print_status "OK" "Documentation $doc found"
    else
        print_status "WARNING" "Documentation $doc not found"
    fi
done

echo ""
echo "🔍 Validating YAML syntax..."

# Check if yamllint is available
if command -v yamllint >/dev/null 2>&1; then
    # Validate workflow YAML files
    for workflow in "${workflows[@]}"; do
        if [ -f "$workflows_dir/$workflow" ]; then
            if yamllint "$workflows_dir/$workflow" >/dev/null 2>&1; then
                print_status "OK" "YAML syntax valid for $workflow"
            else
                print_status "ERROR" "YAML syntax error in $workflow"
                yamllint "$workflows_dir/$workflow"
            fi
        fi
    done
    
    # Validate example configurations
    if [ -d "examples" ]; then
        for example in examples/*.yaml; do
            if [ -f "$example" ]; then
                if yamllint "$example" >/dev/null 2>&1; then
                    print_status "OK" "YAML syntax valid for $(basename "$example")"
                else
                    print_status "ERROR" "YAML syntax error in $(basename "$example")"
                fi
            fi
        done
    fi
else
    print_status "WARNING" "yamllint not available, skipping YAML validation"
fi

echo ""
echo "🔍 Checking ESPHome integration..."

# Run the main test script
if ./test-impulse-cover.sh >/dev/null 2>&1; then
    print_status "OK" "ESPHome component tests pass"
else
    print_status "ERROR" "ESPHome component tests fail"
    echo "Run './test-impulse-cover.sh' for detailed output"
fi

echo ""
echo "🔍 Checking Git configuration..."

# Check gitignore
if [ -f ".gitignore" ]; then
    print_status "OK" ".gitignore found"
    
    # Check for common patterns
    common_patterns=("__pycache__" "*.pyc" ".venv" "build/" "dist/")
    for pattern in "${common_patterns[@]}"; do
        if grep -q "$pattern" .gitignore; then
            print_status "OK" "Gitignore includes $pattern"
        else
            print_status "WARNING" "Gitignore missing $pattern"
        fi
    done
else
    print_status "WARNING" ".gitignore not found"
fi

echo ""
echo "📋 CI/CD Validation Summary:"
echo ""

# Count files
workflow_count=$(find .github/workflows -name "*.yml" 2>/dev/null | wc -l)
template_count=$(find .github/ISSUE_TEMPLATE -name "*.yml" 2>/dev/null | wc -l)
doc_count=0
for doc in "${docs[@]}"; do
    [ -f "$doc" ] && ((doc_count++))
done

echo "   📁 GitHub Workflows: $workflow_count files"
echo "   📝 Issue Templates: $template_count files"
echo "   📚 Documentation: $doc_count files"
echo "   🔧 Configuration: $([ -f "pyproject.toml" ] && echo "✅" || echo "❌") pyproject.toml"
echo "   🐳 Dev Container: $([ -f ".devcontainer/devcontainer.json" ] && echo "✅" || echo "❌") devcontainer.json"
echo "   💻 VS Code Workspace: $([ -f "esphome-impulse-cover.code-workspace" ] && echo "✅" || echo "❌") workspace file"

echo ""
echo "🚀 CI/CD infrastructure validation complete!"
echo ""

# Final recommendations
echo "💡 Next steps:"
echo "   1. Commit and push changes to trigger GitHub Actions"
echo "   2. Create a test pull request to verify workflows"
echo "   3. Check GitHub repository settings match .github/settings.yml"
echo "   4. Enable Dependabot security alerts in repository settings"
echo ""

echo "🔗 Useful commands:"
echo "   - Test locally: ./test-impulse-cover.sh --compile"
echo "   - Format code: black components/ examples/"
echo "   - Lint code: flake8 components/ && pylint components/"
echo "   - Validate YAML: yamllint .github/workflows/ examples/"
