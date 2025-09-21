window.MindMap = class MindMap {
    constructor(nodeManager) {
        this.nodeManager = nodeManager;
        this.svg = d3.select("svg");
        this.width = this.svg.node().clientWidth;
        this.height = this.svg.node().clientHeight;

        this.simulation = d3.forceSimulation()
            .force("link", d3.forceLink().id(d => d.id).distance(100))
            .force("charge", d3.forceManyBody().strength(-300))
            .force("center", d3.forceCenter(this.width / 2, this.height / 2))
            .force("collision", d3.forceCollide().radius(60));

        this.link = this.svg.append("g")
            .attr("class", "links")
            .selectAll(".link");

        this.node = this.svg.append("g")
            .attr("class", "nodes")
            .selectAll(".node");

        // 连接信号槽
        this.setupConnections();
    }

    init() {
        this.updateGraph();
    }

    setupConnections() {
        this.nodeManager.graphUpdated.connect((graph) => {
            this.updateGraph(graph);
        });
    }

    updateGraph(graphData) {
        if (!graphData) {
            // 获取初始数据
            const graph = this.nodeManager.getFullGraph();
            this.renderGraph(graph);
            return;
        }

        this.renderGraph(graphData);
    }

    renderGraph(graphData) {
        const nodes = graphData.nodes;
        const links = graphData.links;

        // 更新链接
        this.link = this.link.data(links, d => `${d.source}-${d.target}`);
        this.link.exit().remove();
        this.link = this.link.enter()
            .append("line")
            .attr("class", "link")
            .merge(this.link);

        // 更新节点
        this.node = this.node.data(nodes, d => d.id);
        this.node.exit().remove();

        const newNode = this.node.enter()
            .append("g")
            .attr("class", "node")
            .call(d3.drag()
                .on("start", (event, d) => this.dragstarted(event, d))
                .on("drag", (event, d) => this.dragged(event, d))
                .on("end", (event, d) => this.dragended(event, d)));

        newNode.append("circle")
            .attr("r", 20);

        newNode.append("text")
            .attr("dy", 4)
            .attr("text-anchor", "middle")
            .text(d => d.text);

        this.node = newNode.merge(this.node);

        // 更新模拟
        this.simulation.nodes(nodes);
        this.simulation.force("link").links(links);
        this.simulation.alpha(0.3).restart();

        // 添加tick函数
        this.simulation.on("tick", () => this.ticked());
    }

    ticked() {
        this.link
            .attr("x1", d => d.source.x)
            .attr("y1", d => d.source.y)
            .attr("x2", d => d.target.x)
            .attr("y2", d => d.target.y);

        this.node
            .attr("transform", d => `translate(${d.x},${d.y})`);
    }

    dragstarted(event, d) {
        if (!event.active) this.simulation.alphaTarget(0.3).restart();
        d.fx = d.x;
        d.fy = d.y;
    }

    dragged(event, d) {
        d.fx = event.x;
        d.fy = event.y;
    }

    dragended(event, d) {
        if (!event.active) this.simulation.alphaTarget(0);
        d.fx = null;
        d.fy = null;
    }
}