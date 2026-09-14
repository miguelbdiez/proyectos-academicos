package servicios;

public class Mensaje {
	
	private int tipoMsj;
	private String idMsj;
	private String contenido;
	private float orden;
	private int flajApertura;
	private int origen;
	
	public Mensaje(int tipoMsj,String idMsj, String contenido, float orden, int flajApertura, int origen) {
		
		this.tipoMsj= tipoMsj;
		this.idMsj= idMsj;
		this.contenido = contenido;
		this.orden = orden;
		this.flajApertura= flajApertura;
		this.origen = origen;
		
		
	}
	




	


	public int getTipoMsj() {
		return tipoMsj;
	}

	public void setTipoMsj(int tipoMsj) {
		this.tipoMsj = tipoMsj;
	}

	public String getIdMsj() {
		return idMsj;
	}

	public void setIdMsj(String idMsj) {
		this.idMsj = idMsj;
	}

	public String getContenido() {
		return contenido;
	}

	public void setContenido(String contenido) {
		this.contenido = contenido;
	}

	public float getOrden() {
		return orden;
	}

	public void setOrden(float orden) {
		this.orden = orden;
	}

	public int getFlajApertura() {
		return flajApertura;
	}

	public void setFlajApertura(int flajApertura) {
		this.flajApertura = flajApertura;
	}

	public int getOrigen() {
		return origen;
	}

	public void setOrigen(int origen) {
		this.origen = origen;
	}
	

}
