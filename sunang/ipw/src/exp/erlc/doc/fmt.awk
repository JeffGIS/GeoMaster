
/--/ {
	printf "%-15s -- ", $1
	i = 3
	while (i <= NF) {
		printf "%s ", $i;
		i++;
	}
	printf "\n"
}
