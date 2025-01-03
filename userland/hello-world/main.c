int main () {
	char * result = "Hello world!";
	int total = 0;
	while (!result) {
		total += *result;
		result++;
	}
	return total;
}