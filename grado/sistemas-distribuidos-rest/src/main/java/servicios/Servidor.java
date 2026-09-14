package servicios;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.URI;
import java.net.URL;
import java.net.UnknownHostException;

import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Random;
import java.util.concurrent.Semaphore;

import javax.inject.Singleton;
import javax.ws.rs.Consumes;
import javax.ws.rs.DefaultValue;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.client.Client;
import javax.ws.rs.client.ClientBuilder;
import javax.ws.rs.client.WebTarget;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.UriBuilder;



@Path("servidor")
@Singleton

public class Servidor {
	
	

	
	Proceso miProceso ;
	int idProc;
	private Object sincronizacionInicio = new Object();
	Semaphore semaforoRegistro = new Semaphore(1);
	Semaphore semBuzon = new Semaphore(1);
	int[] matrizPreparados = new int[3];
	int[] matrizFinalizados = new int[3];
	
	private static HashMap<String, Mensaje> hashMap = new HashMap<>();
	public int relojLamport=0;



	

	
	
	@GET			//tipo de petición HTTP
	@Produces(MediaType.TEXT_PLAIN)	//tipo de texto devuelto
	@Path("hola_Mundo")	//ruta al método
	public String hola_Mundo()	//el método debe retornar String
	{
		String cadena = "hola Mundo";

		return cadena;
	}
	
	@GET			//tipo de petición HTTP
	@Produces(MediaType.TEXT_PLAIN)	//tipo de texto devuelto
	@Path("reinicio")	//ruta al método
	public String start()//el método debe retornar String
	{
		
		reinicio();


		
		return "reinicio";
    
		
		
	}
	
	
	
	private void reinicio() {
		relojLamport=0;
		matrizPreparados[0]=0;		
		matrizPreparados[1]=0;	
		matrizPreparados[2]=0;	

		matrizFinalizados[0]=0;		
		matrizFinalizados[1]=0;	
		matrizFinalizados[2]=0;			
	}

	@GET			//tipo de petición HTTP
	@Produces(MediaType.TEXT_PLAIN)	//tipo de texto devuelto
	@Path("start")	//ruta al método
	public String start(@DefaultValue("0") @QueryParam(value = "id" ) int id,
						@DefaultValue("") @QueryParam("direccion_1") String direccion_1,
						@DefaultValue("") @QueryParam("direccion_2") String direccion_2,
						@DefaultValue("") @QueryParam("direccion_3") String direccion_3)//el método debe retornar String
	{
		
		idProc = id;
		String[] arrayDirecciones_Servidores = {direccion_1, direccion_2,direccion_3};
		miProceso = new Proceso(id,this, arrayDirecciones_Servidores);
		miProceso.preparado(arrayDirecciones_Servidores);
		
		

		return "Preparado";
    
		
		
	}
	
	@GET			//tipo de petición HTTP
	@Produces(MediaType.TEXT_PLAIN)	//tipo de texto devuelto
	@Path("close")	//ruta al método
	public String close()	//el método debe retornar String
	{
		String cadena = "salir_Excribir �:Fichero";
		// miProceso.fin();
		// L�gica para escribir en el fichero despu�s de recibir la notificaci�n
	
		
	
    
		
		
		return cadena;
	}
	

	
	@GET			//tipo de petición HTTP
	@Produces(MediaType.TEXT_PLAIN)	//tipo de texto devuelto
	@Path("buzon")
	public String buzon(@DefaultValue("0") @QueryParam("tipoMsj") int tipoMsj,
            @DefaultValue("") @QueryParam("idMsj") String idMsj,
            @DefaultValue("0") @QueryParam("contenido") String contenido,
            @DefaultValue("0") @QueryParam("orden") float orden,
            @DefaultValue("0") @QueryParam("flajApertura") int flajApertura,
            @DefaultValue("0") @QueryParam("origen") int origen) throws InterruptedException
	
	{
		int instanteLlegada;
		//System.err.print(origen);
		semBuzon.acquire();
		
		
		if( tipoMsj == 0) {
			instanteLlegada = relojLamport;
			relojLamport++;	
			Mensaje msj = new Mensaje(tipoMsj, idMsj, contenido, orden, flajApertura, origen);	
			actualizarRegistro(msj);
				
			String instanteLlegadaString = String.valueOf(instanteLlegada)+"."+String.valueOf(idProc);
			
			semBuzon.release();
			return instanteLlegadaString;
			
		}else if (tipoMsj == 1) {
			instanteLlegada = relojLamport;
			relojLamport++;	

			Mensaje msjApertura = new Mensaje(tipoMsj, idMsj, contenido, orden, flajApertura, origen);
			actualizarRegistro(msjApertura);
			semBuzon.release();
			return "";
		}else if (tipoMsj == 2) {
			
			matrizFinalizados[origen-1] = 1;
				
			if(comprobarMatriz(matrizFinalizados)== 0) {
				miProceso.writeHashMapToFile(hashMap);
				reinicio();
			}

			semBuzon.release();
			
			return "";
		}else if (tipoMsj == 3) {
			
			matrizPreparados[origen-1] = 1;
			

			
			
			if(comprobarMatriz(matrizPreparados)== 0) {
				miProceso.start();
			}
		            
		        
			semBuzon.release();
			
			return "";
				
		}
			
			
		semBuzon.release();
       return "";
			
	}


	private int comprobarMatriz(int[] matriz) {
		for (int i = 0; i < matriz.length; i++) {
            if (matriz[i] != 1) {
            	return 1;
            }
        }
		return 0;	
	}
	
	private void actualizarRegistro(Mensaje msj)
	{	
		
		if(msj.getTipoMsj()== 0) {
			
			hashMap.put(msj.getIdMsj(), msj);
			
		}else if(msj.getTipoMsj() == 1) {
			
			Mensaje actMensaje = hashMap.get(msj.getIdMsj());
			
			if(actMensaje!=null) {
				actMensaje.setOrden(msj.getOrden());
				actMensaje.setFlajApertura(1);	
				hashMap.put(actMensaje.getIdMsj(), actMensaje);
				
			}
			
			
		}


	}
	
}




