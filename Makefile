.PHONY: docker
docker: docker_clean docker_build docker_run

.PHONY: docker_build
docker_build:
	docker build --pull -f Dockerfile -t tarolling/rpsbot "."
.PHONY: docker_clean
docker_clean:
	docker container prune
.PHONY: docker_run
docker_run:
	docker run -it --env-file .env tarolling/rpsbot