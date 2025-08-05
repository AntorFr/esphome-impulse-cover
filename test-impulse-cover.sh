#!/bin/bash
# ESPHome Impulse Cover Component compilation test script

set -e

echo "=== ESPHome Impulse Cover Component Test ==="

# Check if Python3 is installed
if ! command -v python3 &> /dev/null; then
    echo "❌ Python3 is not installed"
    exit 1
fi

# Setup Python virtual environment if needed
if [ ! -d ".venv" ]; then
    echo "📦 Creating Python virtual environment..."
    python3 -m venv .venv
fi

source .venv/bin/activate

# Install/update ESPHome and code quality tools
echo "📦 Installing/updating ESPHome and code quality tools..."
pip install --upgrade esphome > /dev/null 2>&1
pip install --upgrade black isort yamllint pylint > /dev/null 2>&1

# Code Quality Checks Section
echo ""
echo "🎨 === CODE QUALITY CHECKS ==="

# 1. Black formatting check
echo "🖤 Checking Black code formatting..."
if python -m black --check --diff components/ > /dev/null 2>&1; then
    echo "✅ Black formatting: All files properly formatted"
else
    echo "❌ Black formatting: Issues found"
    echo "🔧 Run: python -m black components/ to fix formatting"
    python -m black --check --diff components/
    exit 1
fi

# 2. Import sorting check (isort)
echo "📋 Checking import sorting..."
if python -m isort --check-only --diff components/ > /dev/null 2>&1; then
    echo "✅ Import sorting: All imports properly sorted"
else
    echo "❌ Import sorting: Issues found"
    echo "🔧 Run: python -m isort components/ to fix import sorting"
    python -m isort --check-only --diff components/
    exit 1
fi

# 3. YAML linting check
echo "📝 Checking YAML formatting..."
if yamllint examples/ .github/ > /dev/null 2>&1; then
    echo "✅ YAML linting: All YAML files valid"
else
    echo "❌ YAML linting: Issues found"
    echo "🔧 Fix trailing spaces and formatting issues"
    yamllint examples/ .github/
    exit 1
fi

# 4. Python code quality (pylint)
echo "🐍 Checking Python code quality..."
if python -m pylint components/ --max-line-length=100 --disable=missing-docstring > /dev/null 2>&1; then
    echo "✅ Pylint: Code quality excellent"
else
    echo "❌ Pylint: Code quality issues found"
    echo "🔧 Review and fix pylint warnings"
    python -m pylint components/ --max-line-length=100 --disable=missing-docstring
    exit 1
fi

echo "🎉 All code quality checks passed!"
echo ""
echo "🧪 === ESPHOME CONFIGURATION TESTS ==="

# Create test secrets if they don't exist
if [ ! -f "secrets.yaml" ]; then
    echo "📝 Creating test secrets.yaml..."
    cat > secrets.yaml << EOF
wifi_ssid: "test_network"
wifi_password: "test_password"
EOF
fi

# Check ESPHome version
echo "📋 ESPHome version: $(python3 -m esphome version)"

# Test basic configuration validation
echo "🧪 Testing basic configuration validation..."
if python3 -m esphome config examples/basic-configuration.yaml > /dev/null 2>&1; then
    echo "✅ Basic configuration valid"
else
    echo "❌ Basic configuration invalid"
    python3 -m esphome config examples/basic-configuration.yaml
    exit 1
fi

# Test configuration with sensors validation
echo "🧪 Testing configuration with sensors validation..."
if python3 -m esphome config examples/with-sensors.yaml > /dev/null 2>&1; then
    echo "✅ With sensors configuration valid"
else
    echo "❌ With sensors configuration invalid"
    python3 -m esphome config examples/with-sensors.yaml
    exit 1
fi

# Test partial opening configuration validation
echo "🧪 Testing partial opening configuration validation..."
if python3 -m esphome config examples/partial-test.yaml > /dev/null 2>&1; then
    echo "✅ Partial opening configuration valid"
else
    echo "❌ Partial opening configuration invalid"
    python3 -m esphome config examples/partial-test.yaml
    exit 1
fi

# Component dependency check
echo "🔍 Checking component dependencies..."
if grep -q "impulse_cover" components/impulse_cover/__init__.py > /dev/null 2>&1; then
    echo "✅ Component __init__.py found"
else
    echo "❌ Component __init__.py missing"
    exit 1
fi

if [ -f "components/impulse_cover/cover.py" ]; then
    echo "✅ Component cover.py found"
else
    echo "❌ Component cover.py missing"
    exit 1
fi

if [ -f "components/impulse_cover/impulse_cover.h" ]; then
    echo "✅ Component header file found"
else
    echo "❌ Component header file missing"
    exit 1
fi

if [ -f "components/impulse_cover/impulse_cover.cpp" ]; then
    echo "✅ Component implementation file found"
else
    echo "❌ Component implementation file missing"
    exit 1
fi

# Compilation test (optional - takes time)
if [ "$1" = "--compile" ]; then
    echo "🔨 Testing basic configuration compilation..."
    
    # Create temporary config with correct paths for testing
    cp examples/basic-configuration.yaml temp-basic-config.yaml
    # For local testing, use ./components instead of ../components
    sed -i.bak "s|path: ../components|path: ./components|g" temp-basic-config.yaml
    
    if python3 -m esphome compile temp-basic-config.yaml > /dev/null 2>&1; then
        echo "✅ Basic configuration compilation successful"
    else
        echo "❌ Basic configuration compilation failed"
        python3 -m esphome compile temp-basic-config.yaml
        rm -f temp-basic-config.yaml temp-basic-config.yaml.bak
        exit 1
    fi
    rm -f temp-basic-config.yaml temp-basic-config.yaml.bak

    echo "🔨 Testing with sensors configuration compilation..."
    
    # Create temporary config with correct paths for testing
    cp examples/with-sensors.yaml temp-sensors-config.yaml
    sed -i.bak "s|path: ../components|path: ./components|g" temp-sensors-config.yaml
    
    if python3 -m esphome compile temp-sensors-config.yaml > /dev/null 2>&1; then
        echo "✅ With sensors configuration compilation successful"
    else
        echo "❌ With sensors configuration compilation failed"
        python3 -m esphome compile temp-sensors-config.yaml
        rm -f temp-sensors-config.yaml temp-sensors-config.yaml.bak
        exit 1
    fi
    rm -f temp-sensors-config.yaml temp-sensors-config.yaml.bak

    echo "🔨 Testing partial opening configuration compilation..."
    
    # Create temporary config with correct paths for testing
    cp examples/partial-test.yaml temp-partial-config.yaml
    sed -i.bak "s|path: ../components|path: ./components|g" temp-partial-config.yaml
    
    if python3 -m esphome compile temp-partial-config.yaml > /dev/null 2>&1; then
        echo "✅ Partial opening configuration compilation successful"
    else
        echo "❌ Partial opening configuration compilation failed"
        python3 -m esphome compile temp-partial-config.yaml
        rm -f temp-partial-config.yaml temp-partial-config.yaml.bak
        exit 1
    fi
    rm -f temp-partial-config.yaml temp-partial-config.yaml.bak
fi

# Binary sensor conditional compilation check
echo "🔍 Testing binary sensor conditional compilation..."
if grep -q "#ifdef USE_BINARY_SENSOR" components/impulse_cover/impulse_cover.cpp > /dev/null 2>&1; then
    echo "✅ Binary sensor conditional compilation found"
else
    echo "❌ Binary sensor conditional compilation missing"
    exit 1
fi

# Documentation check
echo "📚 Checking documentation..."
if [ -f "docs/PARTIAL_OPENING.md" ]; then
    echo "✅ Partial opening documentation found"
else
    echo "❌ Partial opening documentation missing"
    exit 1
fi

if [ -f "README.md" ]; then
    echo "✅ README.md found"
else
    echo "❌ README.md missing"
    exit 1
fi

echo "🎉 All tests passed successfully!"
echo ""
echo "📋 Summary:"
echo "   - Code Quality Checks: ✅ All passed"
echo "     • Black formatting: ✅ Perfect"
echo "     • Import sorting: ✅ Perfect"  
echo "     • YAML linting: ✅ Perfect"
echo "     • Pylint score: ✅ Excellent"
echo "   - ESPHome Configuration Tests: ✅ All valid"
echo "     • Basic Configuration: ✅ Valid"
echo "     • With Sensors Configuration: ✅ Valid"
echo "     • Partial Opening Configuration: ✅ Valid"
echo "   - Component Files: ✅ Present"
echo "   - Binary Sensor Support: ✅ Conditional compilation"
echo "   - Documentation: ✅ Complete"
if [ "$1" = "--compile" ]; then
    echo "   - Basic Compilation: ✅ Successful"
    echo "   - With Sensors Compilation: ✅ Successful"
    echo "   - Partial Opening Compilation: ✅ Successful"
fi
echo ""
echo "🚀 Impulse Cover component is ready for use!"
echo ""
echo "🎯 This script validates:"
echo "   - Code quality (Black, isort, yamllint, pylint)"
echo "   - ESPHome configuration syntax"
echo "   - Component file structure"
echo "   - Documentation completeness"
echo ""
echo "💡 Usage examples:"
echo "   - Basic setup: See examples/basic-configuration.yaml"
echo "   - With sensors: See examples/with-sensors.yaml"
echo "   - Partial opening: See examples/partial-test.yaml"
echo "   - Documentation: docs/PARTIAL_OPENING.md"
echo ""
echo "🔧 Options:"
echo "   ./test-impulse-cover.sh          # Full validation (recommended before commit)"
echo "   ./test-impulse-cover.sh --compile # Add compilation tests (slower)"
