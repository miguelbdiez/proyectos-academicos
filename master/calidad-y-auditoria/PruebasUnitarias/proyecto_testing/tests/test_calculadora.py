import pytest
from src.calculadora import Calculadora

# Fixture para crear una instancia de calculadora
@pytest.fixture
def calc():
    """Proporciona una instancia de Calculadora para los tests"""
    return Calculadora()

# Tests de suma
def test_sumar_positivos(calc):
    """Test: Sumar dos números positivos"""
    resultado = calc.sumar(3, 5)
    assert resultado == 8

def test_sumar_negativos(calc):
    """Test: Sumar dos números negativos"""
    resultado = calc.sumar(-3, -5)
    assert resultado == -8

# Tests de resta
def test_restar(calc):
    """Test: Restar dos números"""
    resultado = calc.restar(10, 3)
    assert resultado == 7

# Tests de multiplicación
def test_multiplicar(calc):
    """Test: Multiplicar dos números"""
    resultado = calc.multiplicar(4, 5)
    assert resultado == 20

def test_multiplicar_por_cero(calc):
    """Test: Multiplicar por cero"""
    resultado = calc.multiplicar(5, 0)
    assert resultado == 0

# Tests de división
def test_dividir(calc):
    """Test: Dividir dos números"""
    resultado = calc.dividir(10, 2)
    assert resultado == 5

def test_dividir_por_cero(calc):
    """Test: Dividir por cero debe lanzar excepción"""
    with pytest.raises(ValueError, match="No se puede dividir por cero"):
        calc.dividir(10, 0)

# Test parametrizado
@pytest.mark.parametrize("a,b,esperado", [
    (2, 3, 5),
    (0, 0, 0),
    (-1, 1, 0),
    (100, 200, 300),
])
def test_sumar_parametrizado(calc, a, b, esperado):
    """Test parametrizado: Múltiples casos de suma"""
    assert calc.sumar(a, b) == esperado