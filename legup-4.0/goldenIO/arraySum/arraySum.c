unsigned arraySum (int a[], unsigned size, signed short center){

int i, sum;
	for (i = 0, sum = 0; i< size; i++){
		sum+=a[i];
	}
	return sum;
	if (sum <= center)
		return (center - sum);
	else
		return (sum - center);

}
