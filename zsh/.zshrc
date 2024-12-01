export ZSH="$HOME/.oh-my-zsh"

# ZSH_THEME="robbyrussell"
# Powerlevel10k, Pure, Spaceship, Starship
# agnoster

case "$TERM_PROGRAM" in
  'iTerm.app')
    ZSH_THEME="agnoster"
    ;;
  default)
    ZSH_THEME=""
    ;;
esac


# CASE_SENSITIVE="true"

# HYPHEN_INSENSITIVE="true"

# zstyle ':omz:update' mode disabled  # disable automatic updates
# zstyle ':omz:update' mode auto      # update automatically without asking
# zstyle ':omz:update' mode reminder  # just remind me to update when it's time
# zstyle ':omz:update' frequency 13

# DISABLE_MAGIC_FUNCTIONS="true"

# DISABLE_LS_COLORS="true"

# DISABLE_AUTO_TITLE="true"

# ENABLE_CORRECTION="true"

# COMPLETION_WAITING_DOTS="true"

# DISABLE_UNTRACKED_FILES_DIRTY="true"

# HIST_STAMPS="mm/dd/yyyy"

# ZSH_CUSTOM=/path/to/new-custom-folder

plugins=(git)

source $ZSH/oh-my-zsh.sh

alias ll="ls -la"

alias now="date +%T"

alias py="python"
alias pip="pip3"

alias cve="python -m venv venv"
alias ave="source  /Users/krishtal/Developer/.venv/bin/activate"
alias dve="deactivate"

alias dev="cd ~/Developer"
alias cfg="nvim ~/.config/nvim/init.lua"
alias home="cd ~/"

#THIS MUST BE AT THE END OF THE FILE FOR SDKMAN TO WORK!!!
export SDKMAN_DIR="$HOME/.sdkman"
[[ -s "$HOME/.sdkman/bin/sdkman-init.sh" ]] && source "$HOME/.sdkman/bin/sdkman-init.sh"
