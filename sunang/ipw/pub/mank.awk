
BEGIN {
	vi = 0;
	lastfound = 0;
      }


{
	if ($2 ~ /--/) {
			if (lastfound == 1) {
				lastfound = 0;
				for (x=0; x < vi; x++)
				  print vara[x];
			}
			vi = 0;
	          }


	if ($0 ~ keyword) {  lastfound = 1;  }

	vara[vi++] = $0;

	if ( ($1 ~ /^ipw$/) || ($0 ~ /^ *$/) ) {
		vi--;
		if ( ($2 ~ /--/) && ($3 ~ /^list$/) ) {
			vi++;
		}
	}
}


END {
	if (lastfound == 1) {
		for (x=0; x < vi; x++)
		  print vara[x];
	}
    }
