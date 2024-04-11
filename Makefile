.PHONY: docker
docker: docker_clean docker_build docker_run

.PHONY: docker_build
docker_build:
	docker build --pull -f Dockerfile -t tarolling/rpsbot "."
# If you have containers or images you want to keep,
# do not run this recipe!
.PHONY: docker_clean
docker_clean:
	docker container prune -f
	docker image prune -af
.PHONY: docker_run
docker_run:
	docker run -it tarolling/rpsbot
.PHONY: run
run:
	./build/discord_bot -dev