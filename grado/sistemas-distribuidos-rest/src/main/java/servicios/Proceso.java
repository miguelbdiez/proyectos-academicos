package servicios;

import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Random;

import javax.ws.rs.client.Client;
import javax.ws.rs.client.ClientBuilder;
import javax.ws.rs.client.WebTarget;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.UriBuilder;

import org.glassfish.jersey.internal.util.collection.Value;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.URI;
import java.sql.Date;
import java.text.SimpleDateFormat;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class Proceso extends Thread{
	
	int NUM_MENSAJES= 10;
	int NUM_PROCESOS = 2;
	
	//ArrayList<String> serverList = new ArrayList<String>();
	private static int id;
	private Servidor miServidor;
	private String miServidorString;
	private String[] arrayDirecciones_Servidores;
	private static HashMap<String, Mensaje> hashMap = new HashMap<>();
	private static String[] tiemposDepurar = new String[30] ;
	private int indice=0;
	WebTarget[] clientes;
	

	
	PrintWriter writer;
	
	
	public Proceso( int id, Servidor miServidor, String[] arrayDirecciones_Servidores) {
		
		this.id= id;
		this.miServidor = miServidor;
		this.arrayDirecciones_Servidores = arrayDirecciones_Servidores;
	}
	
	public void run()
	{
				
		int tipoMensaje;
		String idMsj;
		String contenido;
		float orden;
		int flajApertura;	
		float tiempoApertura;
		
		Mensaje mensajeMultidifusion;
		

		for(int i = 1; i <= NUM_MENSAJES; i++){
			
			tipoMensaje = 0;
			idMsj = crearId(i);
			contenido= "Mensaje con ID: "+idMsj+" ORIGEN:"+ String.valueOf(id);
			orden =  (float) 0.0 ;
			flajApertura = 0;

			mensajeMultidifusion = new Mensaje(tipoMensaje, idMsj, contenido, orden, flajApertura, id);
			
			tiempoApertura = multidifusion(clientes, mensajeMultidifusion);
			
			
			
						
						
			tipoMensaje = 1;
			contenido= "Mensaje con ID: "+idMsj+" Source: "+ String.valueOf(id);
			orden = tiempoApertura;
			flajApertura = 1;
			
			mensajeMultidifusion = new Mensaje(tipoMensaje, idMsj, contenido, orden, flajApertura, id);
			 
			tiempoApertura = multidifusion(clientes, mensajeMultidifusion);

	
			esperaAletoriaMultidifusion();

		}
		
		//Mensajes de finalizacion 
		tipoMensaje = 2;
		idMsj = "ID: PROC"+id ;
		contenido= "MENSAJE DE FINALIZACION ("+ String.valueOf(id) + ")";
		orden = 0;
		flajApertura = 0;
		
		mensajeMultidifusion = new Mensaje(tipoMensaje, idMsj, contenido, orden, flajApertura, id);

	
		tiempoApertura = multidifusion(clientes, mensajeMultidifusion);
		

	}
		
	

		
		
		

	
	
	private float multidifusion(WebTarget[] clientes, Mensaje mensajeMultidifusion) {


		if(mensajeMultidifusion.getTipoMsj()==0) {
			
			float[] tiemposLLegada = new float[clientes.length];
			float timepoMaximoLlegada;
			int id_Aleatorio;
			int posClienteAleatorio;
			String tiempoString;
			
			int[] posiciones= new int[clientes.length];
			
			for(int k = 0; k<clientes.length; k++) {
				posiciones[k] = k+1; 
			}
			
			for(int i = 0 ; i < clientes.length; i++){
				
				
				id_Aleatorio = elegirCliente(posiciones);
				posClienteAleatorio = id_Aleatorio-1;
				posiciones = eliminarPosicion(posiciones, id_Aleatorio);
			//	mensajeMultidifusion.setContenido(mensajeMultidifusion.getContenido()+ (id_Aleatorio));
				
				tiempoString = clientes[posClienteAleatorio]
					    .path("rest")
					    .path("servidor")
					    .path("buzon")
					    .queryParam("tipoMsj", mensajeMultidifusion.getTipoMsj())
					    .queryParam("idMsj", mensajeMultidifusion.getIdMsj())
					    .queryParam("contenido", mensajeMultidifusion.getContenido())
					    .queryParam("orden", mensajeMultidifusion.getOrden())
					    .queryParam("flajApertura", mensajeMultidifusion.getFlajApertura())
					    .queryParam("origen", mensajeMultidifusion.getOrigen())
					    .request(MediaType.TEXT_PLAIN)
					    .get(String.class);
				
				tiemposLLegada[i]=Float.parseFloat(tiempoString);
				esperaAleatoriaEntreMsj();
						
			}
			
			timepoMaximoLlegada = encontrarMaximo(tiemposLLegada);
			
			return timepoMaximoLlegada;
			

		}else if(mensajeMultidifusion.getTipoMsj() == 1 || mensajeMultidifusion.getTipoMsj() == 2 || mensajeMultidifusion.getTipoMsj() == 3){
			
			String tiempoString;
				
			for(int i = 0 ; i< clientes.length; i++){
				
					
				tiempoString = clientes[i]
					    .path("rest")
					    .path("servidor")
					    .path("buzon")
					    .queryParam("tipoMsj", mensajeMultidifusion.getTipoMsj())
					    .queryParam("idMsj", mensajeMultidifusion.getIdMsj())
					    .queryParam("contenido", mensajeMultidifusion.getContenido())
					    .queryParam("orden", mensajeMultidifusion.getOrden())
					    .queryParam("flajApertura", mensajeMultidifusion.getFlajApertura())
					    .queryParam("origen", mensajeMultidifusion.getOrigen())
					    .request(MediaType.TEXT_PLAIN)
					    .get(String.class);
				
			
				
				}
			
			return 0;
		
		}
			
		
		return 0;
	}

	private float encontrarMaximo(float[] tiemposLLegada) {
		if (tiemposLLegada.length == 0) {
            throw new IllegalArgumentException("El array no puede estar vacío");
        }

        // Inicializar la variable para almacenar el elemento mayor
        float mayor = tiemposLLegada[0];

        // Iterar sobre el array para encontrar el elemento mayor
        for (int i = 1; i < tiemposLLegada.length; i++) {
            if (tiemposLLegada[i] > mayor) {
                mayor = tiemposLLegada[i];
            }
        }

        return mayor;	
	}

	
	private int[] eliminarPosicion(int[] posiciones, int ClienteAleatorio) {
		 int[] nuevoArray = new int[posiciones.length - 1];
        
        // Buscar el elemento en el array
        int indiceElemento = -1;
        for (int i = 0; i < posiciones.length; i++) {
            if (posiciones[i] == ClienteAleatorio) {
                indiceElemento = i;
                break;
            }
        }
        //en caso de que no se encuentre en el array
        if (indiceElemento == -1) {
            return posiciones;
        }
        
        int indiceNuevoArray = 0;
        for (int i = 0; i < posiciones.length; i++) {
            if (i != indiceElemento) {
                nuevoArray[indiceNuevoArray] = posiciones[i];
                indiceNuevoArray++;
            }
        }
        
        return nuevoArray;

	}

	private int elegirCliente(int[] posiciones) {
		if (posiciones.length == 0) {
            throw new IllegalArgumentException("El array no puede estar vacío");
        }		
		
        Random random = new Random();
        int indiceAEliminar = random.nextInt(posiciones.length);
        int elementoEliminado = posiciones[indiceAEliminar];

        return  elementoEliminado;
	}

	private WebTarget[] crearClientes(String[] arrayDirecciones_Servidores) {
		// TODO Auto-generated method stub
		
        WebTarget[] clientes = new WebTarget[arrayDirecciones_Servidores.length];

		
		for(int i = 0 ; i < arrayDirecciones_Servidores.length ;  i++) {
			
			Client cliente=ClientBuilder.newClient();
			URI uri=UriBuilder.fromUri(arrayDirecciones_Servidores[i]).build();
			clientes[i] = cliente.target(uri);
				
		}
		
		return clientes;
	}
		
	

	private String crearId(int numMensaje) 
	{
		
		String idMsj = String.valueOf(id)+ "." +String.valueOf(numMensaje);
		return idMsj;
		
	}
	
	
		
		
	
	
	public static void esperaAletoriaMultidifusion() {
        Random random = new Random();
        // Genera un número aleatorio entre 1000 (1 segundo) y 1500 (1.5 segundos)
        int tiempoEnMilisegundos = random.nextInt(500) + 1000;
        try {
            // Pausa la ejecución del hilo actual durante el tiempo aleatorio generado
            Thread.sleep(tiempoEnMilisegundos);
        } catch (InterruptedException e) {
            // Manejo de la excepción en caso de interrupción del hilo durante la espera
            e.printStackTrace();
        }
    }
	

	public static void esperaAleatoriaEntreMsj() {
	        Random random = new Random();
	        // Genera un número aleatorio entre 200 (0.2 segundos) y 500 (0.5 segundos)
	        int tiempoEnMilisegundos = random.nextInt(300) + 200;
	        try {
	            // Pausa la ejecución del hilo actual durante el tiempo aleatorio generado
	            Thread.sleep(tiempoEnMilisegundos);
	        } catch (InterruptedException e) {
	            // Manejo de la excepción en caso de interrupción del hilo durante la espera
	            e.printStackTrace();
	        }
	    }
	
	public static void writeHashMapToFile(HashMap<String, Mensaje> hashMap) {
	
		    String filePath = crearFicheroLog(id);      
	        HashMap<String, Mensaje> hashMapOrdenado = ordenarPorOrden(hashMap);

	        
	       int i = 1;
	        
	        try (PrintWriter writer = new PrintWriter(new FileWriter(filePath))) {
	            // Escribir la cabecera en el archivo
	            writer.println("MSJ	|	IDENTIFICADOR MENSAJE	|	ORDEN APERTURA	|		CONTENIDO");
	            // Escribir cada elemento del HashMap en una línea separada en el archivo
	            for (Entry<String, Mensaje> entry : hashMapOrdenado.entrySet()) {
	                Mensaje mensaje = entry.getValue();
	                writer.println(i + "	|		"+ mensaje.getIdMsj() + "		|		" + mensaje.getOrden() + "	|	" +  mensaje.getContenido());
	                i++;
	            }
	            System.out.println("HashMap escrito en el archivo de registro correctamente.");
	        } catch (IOException e) {
	            System.err.println("Error al escribir el HashMap en el archivo de registro: " + e.getMessage());
	        }
         
         
     }
    
	 private static String crearFicheroLog(int id) {
		 String directoryPath = "C:/Users/mbarr/OneDrive/Escritorio/4 CARRERA/2º CUATRIMESTRE/SSDD/"; // Ruta del directorio deseado    
	     LocalDateTime hoy = LocalDateTime.now(); 
	        
	     String fecha= String.valueOf(String.valueOf(hoy.getSecond()) +"s;"+
   			  String.valueOf(hoy.getMinute()) +"m;"+
   			  String.valueOf(hoy.getHour()) +"h;"+
   			  String.valueOf(hoy.getDayOfMonth()) +"d;"+
   			  String.valueOf(hoy.getMonth()) +";"+
   			  String.valueOf(hoy.getYear())+"") ;
	     
	        String fileName = "logProceso_" + String.valueOf(id) +"_"+fecha+".txt";
	        String filePath = directoryPath + fileName; 
		return filePath;
	}

	public static HashMap<String, Mensaje> ordenarPorOrden(HashMap<String, Mensaje> hashMap) {
	        // Convertir el HashMap a una lista de entradas
	        List<Map.Entry<String, Mensaje>> listaEntradas = new LinkedList<>(hashMap.entrySet());

	        // Ordenar la lista de entradas según el atributo "orden" de cada mensaje
	        Collections.sort(listaEntradas, new Comparator<Map.Entry<String, Mensaje>>() {
	            @Override
	            public int compare(Map.Entry<String, Mensaje> entrada1, Map.Entry<String, Mensaje> entrada2) {
	                float orden1 = entrada1.getValue().getOrden();
	                float orden2 = entrada2.getValue().getOrden();
	                return Float.compare(orden1, orden2);
	            }
	        });

	        // Crear un nuevo HashMap ordenado
	        HashMap<String, Mensaje> hashMapOrdenado = new LinkedHashMap<>();
	        for (Map.Entry<String, Mensaje> entrada : listaEntradas) {
	            hashMapOrdenado.put(entrada.getKey(), entrada.getValue());
	        }

	        return hashMapOrdenado;
	    }
	
	private static void depurar(String mensajeDepuracion, String filePath) {
		try (PrintWriter writer = new PrintWriter(new FileWriter(filePath))) {            // Crear un TreeMap para ordenar el HashMap por el campo "orden"

            writer.println("HA llegado RUN id: " + mensajeDepuracion);

           
            System.out.println("HashMap escrito en el archivo correctamente.");
        } catch (IOException e) {
            System.err.println("Error al escribir en el archivo: " + e.getMessage());
        }
    }
	
	public static int timepoAperturaMax(int[] arrayTiempos) {
	        // Verificar si el array es nulo o vacío
	        if (arrayTiempos == null || arrayTiempos.length == 0) {
	            throw new IllegalArgumentException("El array proporcionado es nulo o vacío");
	        }

	        // Inicializar el máximo con el primer elemento del array
	        int maximo = arrayTiempos[0];

	        // Iterar sobre el resto del array para encontrar el máximo
	        for (int i = 1; i < arrayTiempos.length; i++) {
	            if (arrayTiempos[i] > maximo) {
	                maximo = arrayTiempos[i];
	            }
	        }

	        // Devolver el máximo encontrado
	        return maximo;
	    }
	
	
		
	
	

	public void preparado(String[] arrayDirecciones_Servidores) {
		
		 clientes = crearClientes(arrayDirecciones_Servidores);
		 
		 
		Mensaje mensajeMultidifusion = new Mensaje(3, "", "", 0, 0, id);
		float tiempoApertura = multidifusion(clientes, mensajeMultidifusion);
		
		
		return;


		 
		
	}

		
	
}
	
	
	


	
	
	

	
	
	
	
	
		
			
		
		

	
	
	
	
	
	

	
	
	
	
	
	
	
	
	
	
	
	
	


