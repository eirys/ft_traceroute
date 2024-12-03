# ============================================================================ #
#                                    TARGETS                                   #
# ============================================================================ #

# ------------------------------- FINAL BINARY ------------------------------- #
NAME		:=	ft_traceroute

# ------------------------------ DIRECTORY NAMES ----------------------------- #
SRC_DIR		:=	src
OBJ_DIR		:=	obj

# ------------------------------ SUBDIRECTORIES ------------------------------ #
UTILS_DIR	:=	utils
INPUT_DIR	:=	input
NETWORK_DIR	:=	network
SYSTEM_DIR	:=	sys

SUBDIRS		:=	$(UTILS_DIR) \
				$(INPUT_DIR) \
				$(SYSTEM_DIR) \
				$(NETWORK_DIR)

OBJ_SUBDIRS	:=	$(addprefix $(OBJ_DIR)/,$(SUBDIRS))
INC_SUBDIRS	:=	$(addprefix $(SRC_DIR)/,$(SUBDIRS))

# ---------------------------------- SOURCES --------------------------------- #
SRC_FILES	:=	main.c \
				$(NETWORK_DIR)/send_request.c \
				$(NETWORK_DIR)/wait_response.c \
				$(NETWORK_DIR)/raw_socket.c \
				$(INPUT_DIR)/options.c \
				$(INPUT_DIR)/callbacks.c \
				$(SYSTEM_DIR)/signal_handlers.c \
				$(SYSTEM_DIR)/wrapper.c \
				$(UTILS_DIR)/log.c

SRC			:=	$(addprefix $(SRC_DIR)/,$(SRC_FILES))
OBJ			:=	$(addprefix $(OBJ_DIR)/,$(SRC_FILES:.c=.o))
DEP			:=	$(addprefix $(OBJ_DIR)/,$(SRC_FILES:.c=.d))

# -------------------------------- COMPILATION ------------------------------- #
## Compiler
CXX			:=	gcc

## Flags
MACROS		:=

INCLUDES	:=	$(addprefix -I./,$(INC_SUBDIRS)) \
				-I./libft

CFLAGS		:=	-MMD \
				-MP \
				-std=gnu17

# Enable modes ------
# debug := 1
error := 1
# optimize := 1

ifdef error
CFLAGS		+=	-Wall \
				-Wextra \
				-Werror
endif

ifdef optimize
CFLAGS		+=	-O3
endif

ifdef debug
CFLAGS		+=	-g
MACROS		+=	__DEBUG
IS_DEBUG	:= " (debug mode)"
endif

DEFINES		:=	$(addprefix -D,$(MACROS))

CFLAGS		+=	$(INCLUDES) \
				$(DEFINES)

LDFLAGS		:=	-lm

# ----------------------------------- MISC ----------------------------------- #
RM			:=	rm -rf

LIBFT 		:=	libft/libft.a
COMPOSE		:=	docker compose
CONTAINER	:=	debian42

# ============================================================================ #
#                                     RULES                                    #
# ============================================================================ #

.PHONY: all
all: $(LIBFT) $(NAME) copy

$(LIBFT):
	@make -sC libft

.PHONY: trace_output
trace_output:
	mkdir -p trace_output/

.PHONY: copy
copy: trace_output
	cp $(NAME) trace_output/$(NAME)

-include $(DEP)

# Compile binary
$(NAME):   $(OBJ)
	@$(CXX) $(CFLAGS) $(OBJ) -o $(NAME) $(LDFLAGS) $(LIBFT)
	@echo "\`$(NAME)\` successfully created."

# Compile obj files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR) $(OBJ_SUBDIRS)
	@echo "Compiling file $<"$(IS_DEBUG)"..."
	@$(CXX) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@make -sC libft clean
	@$(RM) $(OBJ_DIR)
	@echo "Cleaning object files and dependencies."

.PHONY: fclean
fclean: clean
	@make -sC libft fclean
	@$(RM) $(NAME)
	@echo "Removed $(NAME)."

.PHONY: re
re: fclean all

# ---------------------------------- DOCKER ---------------------------------- #

.PHONY: up
# up: graphics_permission
up: trace_output graphics_permission
	$(COMPOSE) up -d

.PHONY: graphics_permission
graphics_permission:
	@xhost +local:docker

.PHONY: run
run: up
	docker exec -ti $(CONTAINER) bash

.PHONY: down
down:
	$(COMPOSE) down

.PHONY: clear
clear: down
	docker volume rm -f $(shell docker volume ls -q)
	docker image rm -f $(shell docker image ls -aq)

.PHONY: fclear
fclear: clear
	docker system prune -f

.PHONY: dre
dre: clear up
