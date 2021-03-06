package main

import (
	"flag"
	"log"
	"os"

	"crypto/tls"

	"github.com/zshipko/worm"

	imaged "github.com/zshipko/imaged/go"
)

func main() {
	wd, _ := os.Getwd()
	root := flag.String("root", wd, "Root path")
	addr := flag.String("addr", "127.0.0.1:9991", "Address to listen on")
	certFile := flag.String("cert", "", "SSL certificate path")
	keyFile := flag.String("key", "", "SSL key path")
	genSSL := flag.Bool("gen", false, "Generate SSL certificate")
	flag.Parse()

	db, err := imaged.Open(*root)
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	ctx := imaged.Context{DB: db}

	var ssl *tls.Config
	if *certFile != "" && *keyFile != "" {
		ssl, err = worm.LoadX509KeyPair(*certFile, *keyFile)
		if err != nil {
			log.Fatal("Unable to load SSL cert: ", err)
		}
	} else if *genSSL {
		ssl, err = worm.GenerateSelfSignedSSLCert("imaged")
		if err != nil {
			log.Fatal("Unable to generate SSL cert: ", err)
		}
	}

	server, err := worm.NewTCPServer(*addr, ssl, &ctx)
	if err != nil {
		log.Fatal(err)
	}

	log.Println("Running server on:", *addr)
	log.Println("DB root:", *root)
	log.Fatal(server.Run())
}
