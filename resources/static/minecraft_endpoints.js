const origin = window.location.origin

function getPlayers(callback) {
    fetch(`/apps/blockworld/players`)
        .then(response => response.text())
        .then(text => {
            const players = text.split(/\r?\n/).filter(player => player !== "")
            callback(players)
        })
}

function createPlayer(player_id) {
    fetch(`/apps/blockworld/players/${player_id}`, {method: "put"})
}

function deletePlayer(player_id) {
    fetch(`/apps/blockworld/players/${player_id}`, {method: "delete"})
}

function playerMove(player_id, move_direction) {
    fetch(`/apps/blockworld/players/${player_id}/move`, {method: "post", body: move_direction})
}

function playerPlaceBlock(player_id) {
    fetch(`/apps/blockworld/players/${player_id}/place_block`, {method: "post"})
}

function setPlayerBlock(player_id, block) {
    fetch(`/apps/blockworld/players/${player_id}/block`, {method: "put", body: block})
}

function generateWorld(seed) {
    fetch(`/apps/blockworld/world/generate`, {method: "put", body: seed})
}

function getBlocks(callback) {
    fetch(`/apps/blockworld/blocks`)
        .then(response => response.text())
        .then(text => {
            const blocks = text.split(/\r?\n/).filter(block => block !== "")
            callback(blocks)
        })
}

function getBlockColor(block_name, callback) {
    fetch(`/apps/blockworld/blocks/${block_name}/color`)
    .then(response => response.text())
    .then(color => {
        callback(color)
    })
}
