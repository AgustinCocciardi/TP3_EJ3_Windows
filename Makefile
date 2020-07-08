make :
	gcc pagos.c -o Pagos -l pthread
	gcc asistencia.c -o Asistencia -l pthread
	gcc socios.c -o Principal -l pthread