class Calculadora:
    """Calculadora simple para operaciones básicas"""
    
    def sumar(self, a, b):
        """Suma dos números"""
        return a + b
    
    def restar(self, a, b):
        """Resta dos números"""
        return a - b
    
    def multiplicar(self, a, b):
        """Multiplica dos números"""
        return a * b
    
    def dividir(self, a, b):
        """Divide dos números"""
        if b == 0:
            raise ValueError("No se puede dividir por cero")
        return a / b
    
    