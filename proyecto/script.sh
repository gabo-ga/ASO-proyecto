#/bin/bash
num_estudiantes=$(zenity --entry\
	 --title="Interfaz" \
	--text="ingrese el numero de estudiantes")

cantidad_grupos=$(zenity --entry\
	 --title="Interfaz" \
	 --text="ingrese el numero de grupos")

plazas_por_grupo=$(zenity --entry\
	 --title="Interfaz"\
	--text="ingrese la cantidad de plazas por grupo")
resultados="resultados"
touch $resultados
./a.out $num_estudiantes $cantidad_grupos $plazas_por_grupo >> $resultados

zenity --info\
	--text="Resultado en el archivo $resultados"

