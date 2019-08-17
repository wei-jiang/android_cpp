// generate random base64 jpg image
function gen_img_url() {
    //TODO: make light/dark colors? contrasting combo
    let colors = ['#1abc9c', '#2ecc71', '#3498db', '#9b59b6', '#f1c40f', '#e67e22', '#e74c3c'];

    let options = {};
    options.blocksPerEdge = Math.max(5, 3);
    options.padding = 20;
    options.spacing = 0;
    options.color = getRandomColorPair();
    const SIZE = 400;
    let canvas = document.createElement('canvas');
    let canvasWidth = canvas.width = SIZE;
    let canvasHeight = canvas.height = SIZE;
    let ctx = canvas.getContext('2d');

    let Height = canvasHeight - 2 * options.padding;
    let Width = canvasWidth - 2 * options.padding;
    let squareHeight = Height / options.blocksPerEdge;
    let squareWidth = Width / options.blocksPerEdge;

    //render
    function render() {
        canvas.style.background = options.color.primary;
        fillMatrix(options.color.secondary);
        randFill(options.color.primary);
    }
    //fills entire matrix a single color
    function fillMatrix(color) {
        for (let i = 0; i < options.blocksPerEdge; i++) {
            for (let j = 0; j < options.blocksPerEdge; j++) {
                drawSquare(color, j, i);
            }
        }
    }
    //draws a single color square
    function drawSquare(color, matrixX, matrixY) {
        let x = options.padding + (matrixX * squareWidth);
        let y = options.padding + (matrixY * squareHeight);
        ctx.beginPath();
        ctx.fillStyle = color;
        ctx.rect(x + options.spacing, y + options.spacing, squareWidth - options.spacing * 2, squareHeight - options.spacing * 2);
        ctx.fill();
        ctx.closePath();
    }
    //get a random color pair
    function getRandomColorPair() {
        let colorPrimary = colors[Math.floor(Math.random() * colors.length)];
        let colorSecondary = colors[Math.floor(Math.random() * colors.length)];
        while (colorSecondary === colorPrimary) {
            colorSecondary = colors[Math.floor(Math.random() * colors.length)];
        }
        return { primary: colorPrimary, secondary: colorSecondary };
    }
    //get the location dependant on a number
    function getLocation(number) {
        let a = Math.floor(number / options.blocksPerEdge);
        let b = number % options.blocksPerEdge;
        return [a, b];
    }
    //fill the canvas with blocks at random positions
    function randFill(color) {
        let numBlocksReplace = getRandomInt(options.blocksPerEdge * options.blocksPerEdge * .25, options.blocksPerEdge * options.blocksPerEdge * .75);
        let replacedBlocks = [];
        while (replacedBlocks.length < numBlocksReplace) {
            let rand = getRandomInt(0, options.blocksPerEdge * options.blocksPerEdge - 1);
            if (replacedBlocks.indexOf(rand) === -1) {
                let coordinate = getLocation(rand);
                drawSquare(color, coordinate[1], coordinate[0]);
                replacedBlocks.push(rand);
            }
        }
    }
    //get a random integer between min and max
    function getRandomInt(min, max) {
        min = Math.ceil(min);
        max = Math.floor(max);
        return Math.floor(Math.random() * (max - min + 1)) + min;
    }
    render()
    return canvas.toDataURL('image/jpeg', 1.0);
}

export default gen_img_url;