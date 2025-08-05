#!/bin/bash
# Pre-commit validation script for ESPHome Impulse Cover
# Run this before every commit to ensure code quality

set -e

echo "🚀 === PRE-COMMIT VALIDATION ==="
echo ""

# Ensure virtual environment is active
if [ ! -d ".venv" ]; then
    echo "📦 Creating Python virtual environment..."
    python3 -m venv .venv
fi

source .venv/bin/activate

# Install required tools
echo "📦 Installing/updating code quality tools..."
pip install --upgrade black isort yamllint pylint esphome > /dev/null 2>&1

echo "🎨 Running code quality checks..."
echo ""

# 1. Black formatting check
echo "🖤 Black formatting..."
if python -m black --check --diff components/; then
    echo "✅ Black: All files properly formatted"
else
    echo "❌ Black: Formatting issues found"
    echo "🔧 Auto-fixing with Black..."
    python -m black components/
    echo "✅ Black: Files formatted automatically"
fi

echo ""

# 2. Import sorting check
echo "📋 Import sorting..."
if python -m isort --check-only --diff components/; then
    echo "✅ isort: All imports properly sorted"
else
    echo "❌ isort: Import sorting issues found"
    echo "🔧 Auto-fixing with isort..."
    python -m isort components/
    echo "✅ isort: Imports sorted automatically"
fi

echo ""

# 3. YAML linting
echo "📝 YAML validation..."
if yamllint examples/ .github/ > /dev/null 2>&1; then
    echo "✅ yamllint: All YAML files valid"
else
    echo "❌ yamllint: YAML formatting issues found"
    echo "🔧 Please fix manually:"
    yamllint examples/ .github/
    exit 1
fi

echo ""

# 4. Python code quality
echo "🐍 Python code quality..."
pylint_output=$(python -m pylint components/ --max-line-length=100 --disable=missing-docstring 2>&1)
pylint_score=$(echo "$pylint_output" | grep "rated at" | grep -o "[0-9]*\.[0-9]*" | head -1)

if [[ -z "$pylint_score" ]]; then
    pylint_score="0"
fi

# Use awk for floating point comparison
if awk "BEGIN {exit !($pylint_score >= 9.0)}"; then
    echo "✅ pylint: Code quality excellent (score: $pylint_score/10)"
else
    echo "❌ pylint: Code quality below threshold (score: $pylint_score/10)"
    echo "🔧 Please fix pylint issues:"
    echo "$pylint_output"
    exit 1
fi

echo ""

# 5. ESPHome configuration validation
echo "🧪 ESPHome configuration validation..."
config_count=0
config_passed=0

for config in examples/*.yaml; do
    config_count=$((config_count + 1))
    config_name=$(basename "$config")
    
    if python -m esphome config "$config" > /dev/null 2>&1; then
        echo "✅ $config_name: Valid"
        config_passed=$((config_passed + 1))
    else
        echo "❌ $config_name: Invalid"
        echo "🔧 Please fix configuration issues in $config"
        python -m esphome config "$config"
        exit 1
    fi
done

echo ""
echo "🎉 === PRE-COMMIT VALIDATION COMPLETE ==="
echo ""
echo "📊 Results:"
echo "   • Code quality: ✅ All checks passed"
echo "   • ESPHome configs: ✅ $config_passed/$config_count valid"
echo "   • Ready for commit: ✅ YES"
echo ""
echo "🚀 Your code is ready to be committed!"
