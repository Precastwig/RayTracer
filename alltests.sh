sudo nice -n -10 ./main --type=uniform --rep=100 --file=testfiles/trianglemesh
sudo nice -n -10 ./main --type=nonuniform --rep=100 --file=testfiles/trianglemesh 
sudo nice -n -10 ./main --type=uniform --rep=100 --file=testfiles/both 
sudo nice -n -10 ./main --type=nonuniform --rep=100 --file=testfiles/both 
sudo nice -n -10 ./main --type=uniform --rep=100 --file=testfiles/concentric 
sudo nice -n -10 ./main --type=nonuniform --rep=100 --file=testfiles/concentric 
sudo nice -n -10 ./main --type=uniform --rep=100 --file=testfiles/clumped 
sudo nice -n -10 ./main --type=nonuniform --rep=100 --file=testfiles/clumped 