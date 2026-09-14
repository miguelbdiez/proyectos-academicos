public class Array2 {
    public int findLast (int[] x, int y) {
    //Devuelve el índice del último elemento en x que sea igual a y
    //Si no existe dicho elemento devuelve ‐1
    for (int i=x.length - 1 ; i >= 0; i--){
        if (x[i] == y){
        return i;
        }
    }

    return -1;
    
    }


public static void main(String[] args) {
    int x[]={1, 6, 7};
    int y=1;
    Array2 a=new Array2();
    int resultado = a.findLast (x,y);
    System.out.print("El índice del elemento buscado es: ");
    System.out.println(resultado);
    }
}

